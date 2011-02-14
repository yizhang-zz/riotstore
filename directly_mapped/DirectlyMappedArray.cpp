#include <iostream>
#include <sys/time.h>
#include "../common/Config.h"
#include "../lower/LRUPageReplacer.h"
#include "../lower/BitmapPagedFile.h"
#include "DMADenseIterator.h"
#include "DMASparseIterator.h"
#include "DirectlyMappedArray.h"
#ifdef DTRACE_SDT
#include "riot.h"
#endif

#ifdef PROFILING
int LinearStorage::readCount = 0;
int LinearStorage::writeCount = 0;
double LinearStorage::accessTime = 0.0;
#endif

/// If numElements > 0, create a new array; otherwise read from disk.
/// Whether file exists is ignored.
DirectlyMappedArray::DirectlyMappedArray(const char* fileName, Key_t numElements) 
{
   if (numElements > 0)		// new array to be created
   {
      file = new BitmapPagedFile(fileName, BitmapPagedFile::CREATE);
      buffer = new BufferManager(file, config->dmaBufferSize); 
	  RC_t rc = buffer->allocatePageWithPID(0, headerPage);
      assert(RC_OK & rc);
      // page is already marked dirty
      header = (Header*) headerPage->getImage();
      header->nnz = 0;
      header->endsBy = numElements;
      header->storageType = DMA;
      //Datum_t x;
      //header->dataType = GetDataType(x);
      //header->ch = 'z';
	  headerPage->markDirty();
   }
   else	// existing array
   {
      if (access(fileName, F_OK) != 0)
         throw ("File for array does not exist.");
      file = new BitmapPagedFile(fileName, 0);
      buffer = new BufferManager(file, config->dmaBufferSize); 
      buffer->readPage(0, headerPage);
      header = (Header*) headerPage->getImage();
      assert(header->storageType == DMA);
      //Datum_t x;
      //assert(IsSameDataType(x, header->dataType));
   }
   putcount = 0;
}

DirectlyMappedArray::~DirectlyMappedArray() 
{
	headerPage.reset();
	delete buffer;
	delete file;
}

int DirectlyMappedArray::get(const Key_t &key, Datum_t &datum) 
{
#ifdef PROFILING
    static timeval time1, time2;
    gettimeofday(&time1, NULL);
#endif
	if (key >= header->endsBy) {
		datum = NA_DOUBLE;
		return AC_OutOfRange;
	}

   DMABlock *dab;
   RC_t ret;
   PID_t pid = findPage(key);
   if ((ret=readBlock(pid, &dab)) & RC_FAIL)
	   datum = DMABlock::DefaultValue;
   else {
	   datum = dab->get(key);
	   //buffer->unpinPage(dab->getPageHandle());
	   delete dab;
   }
#ifdef PROFILING
    gettimeofday(&time2, NULL);
    accessTime += time2.tv_sec - time1.tv_sec + (time2.tv_usec - time1.tv_usec)
        / 1000000.0 ;
    readCount++;
#endif
   return AC_OK;
}

int DirectlyMappedArray::batchGet(i64 getCount, Entry *gets)
{
    // assume puts are sorted by key
#ifdef PROFILING
    static timeval time1, time2;
    gettimeofday(&time1, NULL);
#endif
    PID_t lastPid = INVALID_PID;
    DMABlock *dab;

    PID_t newPid;
    i64 nGets = 0;
    for (i64 i = 0; i < getCount; i++) {
		if (gets[i].key >= header->endsBy) {
			*gets[i].pdatum = DMABlock::DefaultValue;
			newPid = INVALID_PID;
		}
		else
			newPid = findPage(gets[i].key);

		if (lastPid != newPid) {
			if (lastPid != INVALID_PID) {
				if (readBlock(lastPid, &dab) & RC_FAIL) {
					for (i64 k = nGets; k > 0; k--)
						*(gets[i-k].pdatum) = DMABlock::DefaultValue;
				}
				else {
					dab->batchGet(nGets, gets + i - nGets);
					//buffer->unpinPage(dab->getPageHandle());
					delete dab;
				}
			}

			nGets = 0;
			lastPid = newPid;
		}
		nGets++;
	}
    // don't forget last block!
	if (nGets > 0 && lastPid != INVALID_PID) {
		if (readBlock(lastPid, &dab) & RC_FAIL) {
			for (i64 k = nGets; k > 0; k--)
				*(gets[getCount-k].pdatum) = DMABlock::DefaultValue;
		}
		else {
			dab->batchGet(nGets, gets + getCount - nGets);
			//buffer->unpinPage(dab->getPageHandle());
			delete dab;
		}
	}

#ifdef PROFILING
    gettimeofday(&time2, NULL);
    accessTime += time2.tv_sec - time1.tv_sec + (time2.tv_usec - time1.tv_usec)
        / 1000000.0 ;
    writeCount++;
#endif
    return AC_OK;
}

int DirectlyMappedArray::batchGet(Key_t beginsAt, Key_t endsBy, std::vector<Entry> &v)
{
	PID_t pid = findPage(beginsAt);
	DMABlock *block = NULL;
	while ((pid-1)*config->dmaBlockCapacity < endsBy) {
		if (readBlock(pid, &block) & RC_OK) {
			block->batchGet(beginsAt, endsBy, v);
			//buffer->unpinPage(dab->getPageHandle());
			delete block;
		}
		pid++;
	}
	return AC_OK;
}

int DirectlyMappedArray::batchPut(std::vector<Entry> &v)
{
    DMABlock *block = NULL;
    std::vector<Entry>::const_iterator begin = v.begin(),
        end = v.end();
    while (begin != end) {
        PID_t pid = findPage(begin->key);
        readOrAllocBlock(pid, &block);
#ifdef DTRACE_SDT
        std::vector<Entry>::const_iterator temp = begin;
#endif
        header->nnz += block->batchPut(begin, end); // begin is updated
#ifdef DTRACE_SDT
        putcount += begin - temp;
        if (putcount >= 1000) {
            RIOT_DMA_PUT();
            putcount -= 1000;
        }
#endif
        delete block;
    }
    return AC_OK;
}

int DirectlyMappedArray::put(const Key_t &key, const Datum_t &datum) 
{
#ifdef PROFILING
	static timeval time1, time2;
    gettimeofday(&time1, NULL);
#endif
	DMABlock *dab;
	PID_t pid = findPage(key);
	int ret = readBlock(pid, &dab);
	if (datum == DMABlock::DefaultValue && ret & RC_FAIL)
		return AC_OK; // delete non-existent

	if (ret & RC_FAIL && newBlock(pid, &dab) & RC_FAIL) {
		Error("cannot read/allocate page %d",pid);
		exit(1);
	}   

	header->nnz += dab->put(key, datum);
#ifdef DTRACE_SDT
    if (++putcount >= 1000) {
        RIOT_DMA_PUT();
        putcount -= 1000;
    }
#endif
	//buffer->markPageDirty(dab->getPageHandle());
	//buffer->unpinPage(dab->getPageHandle());
	delete dab;
#ifdef PROFILING
	gettimeofday(&time2, NULL);
	accessTime += time2.tv_sec - time1.tv_sec + (time2.tv_usec - time1.tv_usec)
		/ 1000000.0 ;
	writeCount++;
#endif
	return AC_OK;
}

int DirectlyMappedArray::batchPut(i64 putCount, const Entry *puts)
{
    // assume puts are sorted by key
#ifdef PROFILING
    static timeval time1, time2;
    gettimeofday(&time1, NULL);
#endif
    PID_t lastPid =  INVALID_PID;
    PID_t newPid;
    DMABlock *dab;
    i64 nPuts = 0;
	bool hasNonDefault = false;
	int ret;
    i64 i;
    for (i = 0; i < putCount; i++)
    {
		if (puts[i].key >= header->endsBy)
			newPid = INVALID_PID;
		else
			newPid = findPage(puts[i].key);

        if (lastPid != newPid) // encountered a new page, ready to write last batch
        {
            if (lastPid != INVALID_PID && hasNonDefault) {
                if ((ret=readOrAllocBlock(lastPid, &dab)) & RC_FAIL)
                    Error("Cannot read or allocate block %d", lastPid);
                header->nnz += dab->batchPut(nPuts, puts + i - nPuts);
#ifdef DTRACE_SDT
                if ((putcount += nPuts) >= 1000) {
                    RIOT_DMA_PUT();
                    putcount -= 1000;
                }
#endif
                //buffer->markPageDirty(dab->getPageHandle());
                //buffer->unpinPage(dab->getPageHandle());
                delete dab;
            }

            nPuts = 0;
			hasNonDefault = false;
            lastPid = newPid;
        }

		if (puts[i].datum != DMABlock::DefaultValue)
			hasNonDefault = true;
        nPuts++;
    }
    // don't forget put for last block!
    if (lastPid != INVALID_PID && hasNonDefault) {
        if ((ret=readOrAllocBlock(lastPid, &dab)) & RC_FAIL)
            Error("Cannot read or allocate block %d", lastPid);
        header->nnz += dab->batchPut(nPuts, puts + i - nPuts);
#ifdef DTRACE_SDT
        if ((putcount += nPuts) >= 1000) {
            RIOT_DMA_PUT();
            putcount -= 1000;
        }
#endif
        //buffer->markPageDirty(dab->getPageHandle());
        //buffer->unpinPage(dab->getPageHandle());
        delete dab;
    }

#ifdef PROFILING
    gettimeofday(&time2, NULL);
    accessTime += time2.tv_sec - time1.tv_sec + (time2.tv_usec - time1.tv_usec)
        / 1000000.0 ;
    writeCount++;
#endif
    return AC_OK;
}

void DirectlyMappedArray::flush()
{
    buffer->flushAllPages();
}

ArrayInternalIterator *DirectlyMappedArray::createIterator(IteratorType t, Key_t &beginsAt, Key_t &endsBy)
{
   if (t == Dense)
      return new DMADenseIterator(beginsAt, endsBy, this);
   else if (t == Sparse)
      return NULL;
   return NULL;
   // return new DMASparseIterator(beginsAt, endsBy, this);
}

RC_t DirectlyMappedArray::readBlock(PID_t pid, DMABlock** block) 
{
   PageHandle ph;
   RC_t ret;
   *block = NULL;
   if ((ret=buffer->readPage(pid, ph)) & RC_FAIL)
      return ret;
   Key_t CAPACITY = config->dmaBlockCapacity;
   *block = new DMABlock(ph, CAPACITY*(pid-1), CAPACITY*pid, false);
   return RC_OK;
}

RC_t DirectlyMappedArray::readOrAllocBlock(PID_t pid, DMABlock** block) 
{
   PageHandle ph;
   RC_t ret;
   *block = NULL;
   if ((ret=buffer->readOrAllocatePage(pid, ph)) & RC_FAIL)
      return ret;
   Key_t CAPACITY = config->dmaBlockCapacity;
   if ((ret & RC_READ) == RC_READ)
       *block = new DMABlock(ph, CAPACITY*(pid-1), CAPACITY*pid, false);
   else if ((ret & RC_ALLOC) == RC_ALLOC) {
       *block = new DMABlock(ph, CAPACITY*(pid-1), CAPACITY*pid, true);
#ifdef DTRACE_SDT
       RIOT_DMA_NEW_BLOCK();
#endif
   }
   return RC_OK;
}

RC_t DirectlyMappedArray::readNextBlock(PageHandle ph, DMABlock** block) 
{
    //PID_t pid = buffer->getPID(ph);
    *block = NULL;
	PID_t pid = ph->getPid();
    if (config->dmaBlockCapacity*pid >= header->endsBy)
        return RC_OutOfRange;
    RC_t ret;
    if ((ret=buffer->readPage(pid+1, ph)) & RC_FAIL)
        return ret;
   Key_t CAPACITY = config->dmaBlockCapacity;
   *block = new DMABlock(ph, CAPACITY*(pid), CAPACITY*(pid+1), false);
   return RC_OK;
}

RC_t DirectlyMappedArray::newBlock(PID_t pid, DMABlock** block) 
{
   PageHandle ph;
   RC_t ret;
   *block = NULL;
   if ((ret=buffer->allocatePageWithPID(pid, ph)) & RC_FAIL)
      return ret;
   Key_t CAPACITY = config->dmaBlockCapacity;
   *block = new DMABlock(ph, CAPACITY*(pid-1), CAPACITY*pid, true);
#ifdef DTRACE_SDT
   RIOT_DMA_NEW_BLOCK();
#endif
   return RC_OK;
}

Key_t DirectlyMappedArray::getPageLowerBound(PID_t pid) 
{
    return config->dmaBlockCapacity*(pid-1);
}

Key_t DirectlyMappedArray::getPageUpperBound(PID_t pid) 
{
    return config->dmaBlockCapacity*(pid);
}

/*
RC_t DirectlyMappedArray::releaseBlock(DMABlock* block, bool dirty) 
{
    if (dirty) {
        buffer->markPageDirty(block->getPageHandle());
    }
    return buffer->unpinPage((block->getPageHandle()));
}

void *DirectlyMappedArray::getPageImage(PageHandle ph)
{
    return buffer->getPageImage(ph);
}
*/
