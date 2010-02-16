#include "BufferManager.h"
#include "PageReplacer.h"
#include "LRUPageReplacer.h"
#include <iostream>
using namespace std;

// Constructs a BufferManager for a paged storage container with a
// memory buffer that holds a given number of pages.
BufferManager::BufferManager(PagedStorageContainer *s, uint32_t n,
                             PageReplacer *pr) {
    packer = NULL;
    if (pr)
        pageReplacer = pr;
    else
        pageReplacer = new LRUPageReplacer();
    
    numSlots = n;
    storage = s;
    // aligned allocation
    pool = allocPageImage(numSlots);
    if (pool == NULL) {
        fprintf(stderr, "Cannot allocate contiguous memory for buffer pool\n");
        exit(1);
    }
        
    // init free list
    // handles = new PageHandle[numSlots];
    headers = new PageRec[numSlots];
    //freelist = headers;
    for (int i=0; i<numSlots; i++) {
        headers[i].image = (char*)pool+i*PAGE_SIZE;
        pageReplacer->add(headers+i);
    }
    //freelist[numSlots-1].next = NULL;
	
    pageHash = new PageHashMap();
}

// Destructs the BufferManager.  All dirty pages will be flushed.
BufferManager::~BufferManager() {
    flushAllPages();
    for (int i=0; i<numSlots; i++) {
        if (headers[i].unpacked && packer)
            packer->destroyUnpacked(headers[i].unpacked);
    }
    freePageImage(pool);
    //delete[] handles;
    delete[] headers;
    delete pageHash;
    delete pageReplacer;
}

// Creates a new page in buffer (with unintialized content), pins
// it, marks it dirty, and returns the handle.
RC_t BufferManager::allocatePage(PageHandle &ph) {
    RC_t ret;
    PID_t pid;
    PageRec *rec;
    if ((ret=storage->allocatePage(pid)) != RC_OK) {
        fprintf(stderr, "Physical storage cannot allocate page; error %d\n", ret);
        return ret;
    }

	if ((ret=replacePage(rec)) != RC_OK) {
	  return ret;
	}

	rec->pid = pid;
    rec->pinCount = 1;
    rec->dirty = true;
    ph = rec;
    pageHash->insert(PageHashMap::value_type(pid, rec));
    return RC_OK;
}

// Creates a new page with given pid in buffer (with unintialized
// content), pins it, marks it dirty, and returns the handle.  This
// method only works if the implementation of PagedStorageContainer
// supports allocatePageWithPID.
RC_t BufferManager::allocatePageWithPID(PID_t pid, PageHandle &ph) {
    RC_t ret = storage->allocatePageWithPID(pid);
    PageRec *rec;
    if (ret != RC_OK) {
        fprintf(stderr, "Physical storage cannot allocate pid %d, error %d.\n",pid,ret);
        return ret;
    }

    if ((ret=replacePage(rec)) != RC_OK) {
        return ret;
    }
    
    ph = rec;
    rec->pid = pid;
    rec->pinCount = 1;
    rec->dirty = true;
    pageHash->insert(PageHashMap::value_type(pid, rec));
    return RC_OK;
}

// Disposes a buffered page. It should be unpinned already. It will be
// removed from both the buffer and the disk storage.  Dirty bit is
// ignored.
RC_t BufferManager::disposePage(PageHandle ph) {
    RC_t ret;
    PageRec *rec = (PageRec*) ph;
    if ((ret=storage->disposePage(rec->pid)) != RC_OK) {
        fprintf(stderr, "Physical storage cannot dispose pid %d, error %d.\n", rec->pid, ret);
        return ret;
    }
    PageHashMap::iterator it = pageHash->find(rec->pid);
    assert(it != pageHash->end());
    // the page should be already unpinned and added to pageReplacer
    assert(rec->pinCount == 0);
    pageHash->erase(it);
    rec->reset();
    pageReplacer->add(rec);
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
        rec = it->second;
        if (rec->pinCount == 0)
            pageReplacer->remove(rec);
        rec->pinCount++;
        ph = rec;
    }
    else {
        if ((ret=replacePage(rec)) != RC_OK) {
            return ret;
        }
        
        rec->pid = pid;
        rec->dirty = false;
        rec->pinCount = 1;
        pageHash->insert(PageHashMap::value_type(pid, rec));
        ph = rec;
        if (storage->readPage(ph) != RC_OK) {
            fprintf(stderr, "Physical storage cannot read pid %d, error %d.\n", rec->pid, ret);
            return ret;
        }
    }
    return RC_OK;
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
        rec->pinCount++;
        ph = rec;
    }
    else {
        if ((ret=replacePage(rec)) != RC_OK) {
            return ret;
        }
        
        rec->pid = pid;
        rec->dirty = false;
        rec->pinCount = 1;
        pageHash->insert(PageHashMap::value_type(pid, rec));
        ph = rec;
        ret = storage->readPage(ph);
        if (ret != RC_OK) {
            ret = storage->allocatePageWithPID(pid);
            rec->dirty = true;
            return ret;
        }
    }
    return RC_OK;
}

// Marks a pinned page as dirty (i.e., modified).
RC_t BufferManager::markPageDirty(const PageHandle ph) {
    PageRec *rec = (PageRec*) ph;
    PageHashMap::iterator it = pageHash->find(rec->pid);
    assert(it != pageHash->end());
    rec->dirty = true;
    return RC_OK;
}

// Pins a page.  As long as a page has at least one pin, it cannot
// be discarded from the buffer.
RC_t BufferManager::pinPage(const PageHandle ph) {
    // the page must have already been pinned (when it was
    // created)
    PageRec *rec = (PageRec*) ph;
    PageHashMap::iterator it = pageHash->find(rec->pid);
    assert(it != pageHash->end());
    rec->pinCount++;
    return RC_OK;
}

// Unpins a page.  If a page has no pin left, it can be discarded
// from the buffer, in which case the handle will become invalid.
RC_t BufferManager::unpinPage(const PageHandle ph) {
    PageRec *rec = (PageRec*) ph;
    PageHashMap::iterator it = pageHash->find(rec->pid);
    assert(it != pageHash->end());
    rec->pinCount--;
    if (rec->pinCount == 0) {
        // Access to this page has all terminated;
        // Let PageReplacer manager this page
        pageReplacer->add(rec);
    }
    return RC_OK;
}

// Flushes a pinned page to disk, if it is dirty.  A page's dirty
// bit is unset after flushing.
RC_t BufferManager::flushPage(const PageHandle ph) {
    RC_t ret;
    PageRec *rec = (PageRec*) ph;
    PageHashMap::iterator it = pageHash->find(rec->pid);
    assert(it != pageHash->end());
    if (rec->dirty) {
        if (packer && rec->unpacked) {
            packer->pack(rec->unpacked, rec->image);
        }
        if ((ret=storage->writePage(ph)) != RC_OK) {
            fprintf(stderr, "Physical storage cannot write pid %d, error %d.\n", rec->pid, ret);
            return ret;
        }
        rec->dirty = false;
    }
    return RC_OK;
}

// Flushes all dirty pages in the buffer to disk.  Pages' dirty bits
// are unset after flushing.
RC_t BufferManager::flushAllPages() {
    bool good = true;
    RC_t ret;
    for (PageHashMap::iterator it = pageHash->begin();
         it != pageHash->end();
         it++) {
        PageRec *rec = it->second;
        if (rec->dirty) {
            if (packer && rec->unpacked) {
                packer->pack(rec->unpacked, rec->image);
            }
            if ((ret=storage->writePage((PageHandle)rec)) != RC_OK) {
                fprintf(stderr, "Physical storage cannot write pid %d, error %d.\n",rec->pid,ret);
                good = false;
            }
            rec->dirty = false;
        }
    }
    return good ? RC_OK : RC_Failure;
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
    if ((ret=pageReplacer->selectToReplace(bh)) != RC_OK) {
        fprintf(stderr, "Out of memory: cannot allocate page in buffer, error %d.\n", ret);
        return ret;
    }
    if (bh->dirty) {
        if (packer && bh->unpacked)
            packer->pack(bh->unpacked, bh->image);
        if ((ret=storage->writePage((PageHandle)bh)) != RC_OK) {
            fprintf(stderr, "Physical storage cannot write pid %d, error %d.\n",bh->pid,ret);
            return ret;
        }
        bh->dirty = false;
    }
    // remove old mapping in hash table
    pageHash->erase(bh->pid);
    // clear unpacked image
    if (packer && bh->unpacked)
        packer->destroyUnpacked(bh->unpacked);
    bh->reset();
    return RC_OK;
}

PID_t BufferManager::getPID(PageHandle ph)
{
    PageRec *rec = (PageRec*) ph;
    return rec->pid;
}

void *BufferManager::getPageImage(PageHandle ph)
{
    PageRec *rec = (PageRec*) ph;
    return rec->image;
}

void *BufferManager::getUnpackedPageImage(PageHandle ph)
{
    PageRec *rec = (PageRec*) ph;
    if (!rec->unpacked && packer) {
        packer->unpack(rec->image, rec->unpacked);
    }
    return rec->unpacked;
}
