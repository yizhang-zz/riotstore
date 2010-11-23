#include <iostream>
#include <sys/time.h>
#include "../common/Config.h"
#include "../lower/LRUPageReplacer.h"
#include "../lower/BitmapPagedFile.h"
#include "DMADenseIterator.h"
#include "DMASparseIterator.h"
#include "DirectlyMappedArray.h"

#ifdef PROFILING
int LinearStorage::readCount = 0;
int LinearStorage::writeCount = 0;
double LinearStorage::accessTime = 0.0;
#endif

double const DirectlyMappedArray::DefaultValue = 0.0;

/// If numElements > 0, create a new array; otherwise read from disk.
/// Whether file exists is ignored.
DirectlyMappedArray::DirectlyMappedArray(const char* fileName, uint32_t numElements) 
{
   if (numElements > 0)		// new array to be created
   {
      remove(fileName);
      file = new BitmapPagedFile(fileName, BitmapPagedFile::CREATE);
      buffer = new BufferManager(file, config->dmaBufferSize); 
      upper = numElements;
      PageHandle ph;
	  RC_t rc = buffer->allocatePageWithPID(0, ph);
      assert(RC_OK == rc);
      // page is already marked dirty
      DirectlyMappedArrayHeader* header = (DirectlyMappedArrayHeader*)
          (buffer->getPageImage(ph));
      header->endsBy = numElements;
      Datum_t x;
      header->dataType = GetDataType(x);
      header->ch = 'z';
      buffer->markPageDirty(ph);
      buffer->unpinPage(ph);
   }
   else 						// existing array
   {
      if (access(fileName, F_OK) != 0)
         throw ("File for array does not exist.");
      file = new BitmapPagedFile(fileName, 0);
      buffer = new BufferManager(file, config->dmaBufferSize); 
      PageHandle ph;
      buffer->readPage(0, ph);
      DirectlyMappedArrayHeader* header = (DirectlyMappedArrayHeader*)
          (buffer->getPageImage(ph));
      buffer->unpinPage(ph);
      upper = header->endsBy;
      Datum_t x;
      assert(IsSameDataType(x, header->dataType));
   }
}

   /// should delete buffer first, because flushAll() is called in
   /// buffer's destructor, at which time file is updated.
DirectlyMappedArray::~DirectlyMappedArray() 
{
   delete buffer;
   delete file;
}

int DirectlyMappedArray::get(const Key_t &key, Datum_t &datum) 
{
#ifdef PROFILING
    static timeval time1, time2;
    gettimeofday(&time1, NULL);
#endif
	if (key >= upper) {
		datum = DefaultValue;
		return AC_OutOfRange;
	}

   PID_t pid;
   DenseArrayBlock *dab;
   RC_t ret;
   findPage(key, &(pid));
   if ((ret=readBlock(pid, &dab)) != RC_OK)
	   datum = DefaultValue;
   else {
	   datum = dab->get(key);
	   buffer->unpinPage(dab->getPageHandle());
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

int DirectlyMappedArray::batchGet(i64 getCount, KVPair_t *gets)
{
    // assume puts are sorted by key
#ifdef PROFILING
    static timeval time1, time2;
    gettimeofday(&time1, NULL);
#endif
    PID_t lastPid = INVALID_PID;
    DenseArrayBlock *dab;

    PID_t newPid;
    i64 nGets = 0;
    for (i64 i = 0; i < getCount; i++) {
		if (gets[i].key >= upper) {
			*gets[i].datum = DefaultValue;
			newPid = INVALID_PID;
		}
		else
			findPage(gets[i].key, &newPid);

		if (lastPid != newPid) {
			if (lastPid != INVALID_PID) {
				if (readBlock(lastPid, &dab) != RC_OK) {
					for (i64 k = nGets; k > 0; k--)
						*(gets[i-k].datum) = DefaultValue;
				}
				else {
					dab->batchGet(nGets, gets + i - nGets);
					buffer->unpinPage(dab->getPageHandle());
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
		if (readBlock(lastPid, &dab) != RC_OK) {
			for (i64 k = nGets; k > 0; k--)
				*(gets[getCount-k].datum) = DefaultValue;
		}
		else {
			dab->batchGet(nGets, gets + getCount - nGets);
			buffer->unpinPage(dab->getPageHandle());
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

int DirectlyMappedArray::put(const Key_t &key, const Datum_t &datum) 
{
#ifdef PROFILING
	static timeval time1, time2;
    gettimeofday(&time1, NULL);
#endif
	PID_t pid;
	DenseArrayBlock *dab;
	findPage(key, &(pid));
	int ret = readBlock(pid, &dab);
	if (datum == DefaultValue && ret != RC_OK)
		return AC_OK; // delete non-existent

	if (ret != RC_OK && newBlock(pid, &dab) != RC_OK) {
		Error("cannot read/allocate page %d",pid);
		exit(1);
	}   

	dab->put(key, datum);
	buffer->markPageDirty(dab->getPageHandle());
	buffer->unpinPage(dab->getPageHandle());
	delete dab;
#ifdef PROFILING
	gettimeofday(&time2, NULL);
	accessTime += time2.tv_sec - time1.tv_sec + (time2.tv_usec - time1.tv_usec)
		/ 1000000.0 ;
	writeCount++;
#endif
	return AC_OK;
}

int DirectlyMappedArray::batchPut(i64 putCount, const KVPair_t *puts)
{
    // assume puts are sorted by key
#ifdef PROFILING
    static timeval time1, time2;
    gettimeofday(&time1, NULL);
#endif
    PID_t lastPid =  INVALID_PID;
    DenseArrayBlock *dab;
    //findPage(puts[0].key, &pid);
    //if (readOrAllocBlock(pid, &dab) != RC_OK) {
    //    Error("cannot read/allocate page %d",pid);
    //    exit(1);
    //}   

    PID_t newPid;
    i64 nPuts = 0;
	bool hasNonDefault = false;
	int ret;
    for (i64 i = 0; i < putCount; i++)
    {
		if (puts[i].key >= upper)
			newPid = INVALID_PID;
		else
			findPage(puts[i].key, &newPid);
		if (*puts[i].datum != DefaultValue)
			hasNonDefault = true;

        if (lastPid != newPid)
        {
			if (lastPid != INVALID_PID) {
				if ((ret=readBlock(lastPid, &dab)) == RC_OK || hasNonDefault) {
					if (ret != RC_OK)
						newBlock(lastPid, &dab);
					dab->batchPut(nPuts, puts + i - nPuts);
					buffer->markPageDirty(dab->getPageHandle());
					buffer->unpinPage(dab->getPageHandle());
					delete dab;
				}
			}

            nPuts = 0;
			hasNonDefault = false;
            lastPid = newPid;
        }
        nPuts++;
    }
    // don't forget put for last block!
	if (nPuts > 0 && lastPid != INVALID_PID) {
		if ((ret=readBlock(lastPid, &dab)) == RC_OK || hasNonDefault) {
			if (ret != RC_OK)
				newBlock(lastPid, &dab);
			dab->batchPut(nPuts, puts + putCount - nPuts);
			buffer->markPageDirty(dab->getPageHandle());
			buffer->unpinPage(dab->getPageHandle());
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

ArrayInternalIterator *DirectlyMappedArray::createIterator(IteratorType t, Key_t &beginsAt, Key_t &endsBy)
{
   if (t == Dense)
      return new DMADenseIterator(beginsAt, endsBy, this);
   else if (t == Sparse)
      return NULL;
   return NULL;
   // return new DMASparseIterator(beginsAt, endsBy, this);
}

RC_t DirectlyMappedArray::readBlock(PID_t pid, DenseArrayBlock** block) 
{
   PageHandle ph;
   RC_t ret;
   if ((ret=buffer->readPage(pid, ph)) != RC_OK)
      return ret;
   Key_t CAPACITY = DenseArrayBlock::CAPACITY;
   *block = new DenseArrayBlock(this, ph, CAPACITY*(pid-1), CAPACITY*pid);
   return RC_OK;
}

RC_t DirectlyMappedArray::readOrAllocBlock(PID_t pid, DenseArrayBlock** block) 
{
   PageHandle ph;
   RC_t ret;
   if ((ret=buffer->readOrAllocatePage(pid, ph)) != RC_OK)
      return ret;
   Key_t CAPACITY = DenseArrayBlock::CAPACITY;
   *block = new DenseArrayBlock(this, ph, CAPACITY*(pid-1), CAPACITY*pid);
   return RC_OK;
}

RC_t DirectlyMappedArray::readNextBlock(PageHandle ph, DenseArrayBlock** block) 
{
    PID_t pid = buffer->getPID(ph);
    if (DenseArrayBlock::CAPACITY*pid >= upper)
        return RC_OutOfRange;
    RC_t ret;
    if ((ret=buffer->readPage(pid+1, ph)) != RC_OK)
        return ret;
   Key_t CAPACITY = DenseArrayBlock::CAPACITY;
   *block = new DenseArrayBlock(this, ph, CAPACITY*(pid), CAPACITY*(pid+1));
   return RC_OK;
}

RC_t DirectlyMappedArray::newBlock(PID_t pid, DenseArrayBlock** block) 
{
   PageHandle ph;
   RC_t ret;
   if ((ret=buffer->allocatePageWithPID(pid, ph)) != RC_OK)
      return ret;
   Key_t CAPACITY = DenseArrayBlock::CAPACITY;
   *block = new DenseArrayBlock(this, ph, CAPACITY*(pid-1), CAPACITY*pid);
   return RC_OK;
}

RC_t DirectlyMappedArray::releaseBlock(DenseArrayBlock* block, bool dirty) 
{
    if (dirty) {
        buffer->markPageDirty(block->getPageHandle());
    }
    return buffer->unpinPage((block->getPageHandle()));
}

uint32_t DirectlyMappedArray::getPageLowerBound(PID_t pid) 
{
    return DenseArrayBlock::CAPACITY*(pid-1);
}

uint32_t DirectlyMappedArray::getPageUpperBound(PID_t pid) 
{
    return DenseArrayBlock::CAPACITY*(pid);
}

void *DirectlyMappedArray::getPageImage(PageHandle ph)
{
    return buffer->getPageImage(ph);
}
