#include "riot.h"
#include "BufferManager.h"
#include <sys/time.h>
#include "PageReplacer.h"
#include "PageRec.h"
#include "LRUPageReplacer.h"
#include <iostream>
using namespace std;

#ifdef PROFILE_BUFMAN
double BufferManager::accessTime = 0.0;
#endif

void BufferManager::printStat()
{
    cout<<"total pin count="<<totalPinCount<<endl
        <<"unmapped count="<<pageReplacer->size()<<endl
        <<"mapped count="<<numSlots-pageReplacer->size()<<endl;
    pageReplacer->print();
}

// Constructs a BufferManager for a paged storage container with a
// memory buffer that holds a given number of pages.
BufferManager::BufferManager(PagedStorageContainer *s, size_t n,
        PageReplacer *pr)
: storage(s), numSlots(n), ppool(sizeof(Page)), pageDealloc(ppool) {
    totalPinCount = 0;
    //packer = NULL;
    if (pr)
        pageReplacer = pr;
    else
        pageReplacer = new LRUPageReplacer();
    
    // aligned allocation
    pool = allocPageImage(numSlots);
    if (pool == NULL) {
        Error("Cannot allocate contiguous memory for buffer pool");
        exit(1);
    }
        
    // init free list
    // handles = new PageHandle[numSlots];
    headers = new PageRec[numSlots];
    //freelist = headers;
    for (size_t i=0; i<numSlots; i++) {
        headers[i].image = (char*)pool+i*PAGE_SIZE;
        pageReplacer->add(headers+i);
    }
    //freelist[numSlots-1].next = NULL;
	
    pageHash = new PageHashMap();
}

// Destructs the BufferManager.  All dirty pages will be flushed.
BufferManager::~BufferManager() {
    flushAllPages();
#ifdef DEBUG
	for (size_t i=0; i<numSlots; i++)
		assert(headers[i].pinCount == 0);
#endif
    //for (int i=0; i<numSlots; i++) {
    //    if (headers[i].unpacked && packer)
    //        packer->destroyUnpacked(headers[i].unpacked);
    //}
    freePageImage(pool);
    //delete[] handles;
    delete[] headers;
    delete pageHash;
    delete pageReplacer;
}

// Creates a new page in buffer (with unintialized content), pins
// it, marks it dirty, and returns the handle.
RC_t BufferManager::allocatePage(PageHandle &ph) {
#ifdef PROFILE_BUFMAN
	TIMESTAMP(t1);
#endif
    RC_t ret;
    PID_t pid;
    PageRec *rec;
	if ((ret=storage->allocatePage(pid)) & RC_FAIL) {
		Error("Physical storage cannot allocate page; error %d", ret);
#ifdef PROFILE_BUFMAN
		TIMESTAMP(t2);
		accessTime += t2-t1;
#endif
		return ret;
	}

	if ((ret=replacePage(rec)) & RC_FAIL) {
#ifdef PROFILE_BUFMAN
		TIMESTAMP(t2);
		accessTime += t2-t1;
#endif
		return ret;
	}
    //Debug("with page %10d\n", pid);

#ifdef DTRACE_SDT
    RIOT_BM_ALLOC(pid);
#endif
	rec->pid = pid;
    rec->dirty = true;
    ph = make_ph(rec);
    pageHash->insert(PageHashMap::value_type(pid, rec));
#ifdef PROFILE_BUFMAN
	TIMESTAMP(t2);
	accessTime += t2-t1;
#endif
    return RC_ALLOC;
}

// Creates a new page with given pid in buffer (with unintialized
// content), pins it, marks it dirty, and returns the handle.  This
// method only works if the implementation of PagedStorageContainer
// supports allocatePageWithPID.
RC_t BufferManager::allocatePageWithPID(PID_t pid, PageHandle &ph) {
#ifdef PROFILE_BUFMAN
	TIMESTAMP(t1);
#endif
    RC_t ret = storage->allocatePageWithPID(pid);
    PageRec *rec;
    if (ret & RC_FAIL) {
        Error("Physical storage cannot allocate pid %d, error %d",pid,ret);
#ifdef PROFILE_BUFMAN
	TIMESTAMP(t2);
	accessTime += t2-t1;
#endif
        return ret;
    }

    if ((ret=replacePage(rec)) & RC_FAIL) {
#ifdef PROFILE_BUFMAN
	TIMESTAMP(t2);
	accessTime += t2-t1;
#endif
        return ret;
    }
    //Debug("with page %10d\n", pid);
    
#ifdef DTRACE_SDT
    RIOT_BM_ALLOC(pid);
#endif
    rec->pid = pid;
    rec->dirty = true;
	ph = make_ph(rec);
    pageHash->insert(PageHashMap::value_type(pid, rec));
#ifdef PROFILE_BUFMAN
	TIMESTAMP(t2);
	accessTime += t2-t1;
#endif
    return RC_ALLOC;
}

// Disposes a buffered page. It should be unpinned already. It will be
// removed from both the buffer and the disk storage.  Dirty bit is
// ignored.
RC_t BufferManager::disposePage(PageRec *rec) {
#ifdef PROFILE_BUFMAN
	TIMESTAMP(t1);
#endif
    RC_t ret;
    //PageRec *rec = (PageRec*) ph;
    if ((ret=storage->disposePage(rec->pid)) & RC_FAIL) {
        Error("Physical storage cannot dispose pid %d, error %d.\n", rec->pid, ret);
#ifdef PROFILE_BUFMAN
	TIMESTAMP(t2);
	accessTime += t2-t1;
#endif
        return ret;
    }
    PageHashMap::iterator it = pageHash->find(rec->pid);
    assert(it != pageHash->end());
    // the page should be already unpinned and added to pageReplacer
    assert(rec->pinCount == 0);
    pageHash->erase(it);
    rec->reset();
    pageReplacer->add(rec);
#ifdef PROFILE_BUFMAN
	TIMESTAMP(t2);
	accessTime += t2-t1;
#endif
    return RC_OK;
}

// Reads a page into buffer (if it is not already in), pins it, and
// returns the handle.
RC_t BufferManager::readPage(PID_t pid, PageHandle &ph) {
    RC_t ret;
    PageRec *rec;
    // first check if already buffered
    PageHashMap::iterator it = pageHash->find(pid);
    if (it != pageHash->end()) {
#ifdef DTRACE_SDT
        RIOT_BM_READ(pid,0);
#endif
        rec = it->second;
        if (rec->pinCount == 0)
            pageReplacer->remove(rec);
		ph = make_ph(rec);
    }
    else {
#ifdef DTRACE_SDT
        RIOT_BM_READ(pid,1);
#endif
        if ((ret=replacePage(rec)) & RC_FAIL) {
            return ret;
        }
        //Debug("with page %10d\n", pid);
        
        rec->pid = pid;
        if ((ret=storage->readPage(rec)) & RC_FAIL) {
            //Debug("Physical storage cannot read pid %d, error %d", rec->pid, ret);
            rec->pid = INVALID_PID;
            pageReplacer->add(rec);  //recycle
            return ret;
        }
        rec->dirty = false;
		ph = make_ph(rec);
        pageHash->insert(PageHashMap::value_type(pid, rec));
    }
    return RC_READ;
}

RC_t BufferManager::readOrAllocatePage(PID_t pid, PageHandle &ph) {
    RC_t ret;
    PageRec *rec;
    // first check if already buffered
    PageHashMap::iterator it = pageHash->find(pid);
    if (it != pageHash->end()) {
        rec = it->second;
        if (rec->pinCount == 0)
            pageReplacer->remove(rec);
		ph = make_ph(rec);
        ret = RC_READ;
#ifdef DTRACE_SDT
        RIOT_BM_READ(pid,0);
#endif
    }
    else {
        if ((ret=replacePage(rec)) & RC_FAIL) {
            return ret;
        }
        //Debug("with page %10d\n", pid);
        
        rec->pid = pid;
        ret = storage->readPage(rec);
        if (ret & RC_FAIL) {
#ifdef DTRACE_SDT
            RIOT_BM_ALLOC(pid);
#endif
            ret = storage->allocatePageWithPID(pid);
            if (ret & RC_FAIL) {
                rec->pid = INVALID_PID;
                pageReplacer->add(rec);
                Error("Read/Alloc: Physical storage cannot allocate page %u"
                      , pid);
                return ret;
            }
            rec->dirty = true;
            ret = RC_ALLOC;
        }
        else {
#ifdef DTRACE_SDT
            RIOT_BM_READ(pid,1);
#endif
            ret = RC_READ;
        }
		ph = make_ph(rec);
        pageHash->insert(PageHashMap::value_type(pid, rec));
    }
    return ret;
}

// Marks a pinned page as dirty (i.e., modified).
RC_t BufferManager::markPageDirty(PageRec *rec) {
    //PageRec *rec = (PageRec*) ph;
    //PageHashMap::iterator it = pageHash->find(rec->pid);
    //assert(it != pageHash->end());
    rec->dirty = true;
    return RC_OK;
}

// Pins a page.  As long as a page has at least one pin, it cannot
// be discarded from the buffer.
RC_t BufferManager::pinPage(PageRec *rec) {
    // the page must have already been pinned (when it was
    // created)
    //PageRec *rec = (PageRec*) ph;
    //PageHashMap::iterator it = pageHash->find(rec->pid);
    //assert(it != pageHash->end());
    rec->pinCount++;
    totalPinCount++;
    return RC_OK;
}

// Unpins a page.  If a page has no pin left, it can be discarded
// from the buffer, in which case the handle will become invalid.
RC_t BufferManager::unpinPage(PageRec *rec) {
    //PageRec *rec = (PageRec*) ph;
    //PageHashMap::iterator it = pageHash->find(rec->pid);
    //assert(it != pageHash->end());
    rec->pinCount--;
    totalPinCount--;
    if (rec->pinCount == 0) {
        // Access to this page has all terminated;
        // Let PageReplacer manager this page
        pageReplacer->add(rec);
    }
    return RC_OK;
}

// Flushes a pinned page to disk, if it is dirty.  A page's dirty
// bit is unset after flushing.
RC_t BufferManager::flushPage(PageRec *rec) {
    RC_t ret;
    //PageRec *rec = (PageRec*) ph;
    PageHashMap::iterator it = pageHash->find(rec->pid);
    assert(it != pageHash->end());
    if (rec->dirty) {
        //if (packer && rec->unpacked) {
        //    packer->pack(rec->unpacked, rec->image);
        //}
        if ((ret=storage->writePage(rec)) & RC_FAIL) {
            Error("Physical storage cannot write pid %d, error %d", rec->pid, ret);
            return ret;
        }
        rec->dirty = false;
    }
    return RC_OK;
}

// Flushes all dirty pages in the buffer to disk.  Pages' dirty bits
// are unset after flushing.
RC_t BufferManager::flushAllPages() {
    RC_t ret = RC_OK;
    for (PageHashMap::iterator it = pageHash->begin();
         it != pageHash->end();
         it++) {
        PageRec *rec = it->second;
        if (rec->dirty) {
            //if (packer && rec->unpacked) {
            //    packer->pack(rec->unpacked, rec->image);
            //}
            if ((ret=storage->writePage(rec)) & RC_FAIL) {
                Error("Physical storage cannot write pid %d, error %d",rec->pid,ret);
                ret |= RC_FAIL;
            }
            rec->dirty = false;
        }
    }
    // storage also needs to flush its metadata info
    storage->flush();
    return ret & RC_FAIL ? RC_FAIL : RC_OK;
}

void BufferManager::print() {
    for (PageHashMap::iterator it = pageHash->begin();
         it != pageHash->end();
         it++) {
        PageRec *rec = it->second;
        cout<<it->first<<"\t"<<rec->pid<<"\t"
            <<rec->image<<"\t"<<rec->pinCount<<endl;
    }
}

RC_t BufferManager::replacePage(PageRec *&bh)
{
    RC_t ret;
    if ((ret=pageReplacer->selectToReplace(bh)) & RC_FAIL) {
        Error("Out of memory: cannot allocate page in buffer, error %d", ret);
        return ret;
    }
    //Debug("relace page %10d ", bh->pid);
    if (bh->dirty) {
        //if (packer && bh->unpacked)
        //    packer->pack(bh->unpacked, bh->image);
        if ((ret=storage->writePage(bh)) & RC_FAIL) {
            Error("Physical storage cannot write pid %d, error %d",bh->pid,ret);
            return ret;
        }
        bh->dirty = false;
        //Debug("Evicted dirty page %d", bh->pid);
    }
    // remove old mapping in hash table
    pageHash->erase(bh->pid);
    // clear unpacked image
    //if (packer && bh->unpacked)
    //    packer->destroyUnpacked(bh->unpacked);
    bh->reset();
    return RC_OK;
}

/*
void *BufferManager::getUnpackedPageImage(PageHandle ph)
{
#ifdef PROFILING
    static timeval time1, time2;
    gettimeofday(&time1, NULL);
#endif
    PageRec *rec = (PageRec*) ph;
    if (!rec->unpacked && packer) {
        packer->unpack(rec->image, rec->unpacked);
    }
#ifdef PROFILING
    gettimeofday(&time2, NULL);
    accessTime += time2.tv_sec - time1.tv_sec + (time2.tv_usec - time1.tv_usec)
        / 1000000.0 ;
#endif
    return rec->unpacked;
}
*/
