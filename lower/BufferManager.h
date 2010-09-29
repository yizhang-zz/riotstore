#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include <apr_pools.h>
#include <map>
#include "../common/common.h"
#include "PagedStorageContainer.h"
#include "PageRec.h"

// forward declaration
class PageReplacer;

class PagePacker
{
public:
    virtual void pack(void *unpacked, void *packed) = 0;
    virtual void unpack(void *packed, void *&unpacked) = 0;
    virtual void destroyUnpacked(void *&unpacked) = 0;
};

//////////////////////////////////////////////////////////////////////
// Provides memory-buffered access to a PagedStorageContainer.  This
// is the interface that data structures such as BTree and
// DirectlyMappedArray should use.  BTree would call allocatePage and
// disposePage, but never allocatePageWithPID.  DirectlyMappedArray
// would call allocatePageWithPID, but never allocatePage or
// disposePage.
//
// A BufferManager always uses a PageReplacer to implement the
// algorithm for selecting a page to be replaced when the entire
// buffer is full and a new page has to be brought into the buffer.

// template <typename T = LRUPageReplacer>
class BufferManager {
    
public:
#ifdef PROFILING
    static double accessTime;

    static void resetPerfCounts()
    {
        accessTime = 0.0;
    }
#endif

private:

    typedef std::map<PID_t, PageRec*> PageHashMap;

    // The underlying storage.
    PagedStorageContainer *storage;

    // Maximum number of pages the buffer can hold.
    uint32_t numSlots;

    // The buffer memory.  It is implemented as an array of size
    // numSlots.
    PageImage pool;

	// A header for each image
	PageRec *headers;

    // PageHandle *handles;

	/* We maintain three lists for the page images. A list for all
		 pinned pages, a list for all the reclaimable pages (unpinned),
		 and a list for all free pages (not mapped to a disk page). The
		 first list is conceptual and does not have to be represented by
		 any actual data structure. The first two lists are hash indexed
		 for quick lookup by PID. The second list is maintained by the
		 PageReplacer class because the PageReplacer gets to choose which
		 page to be swapped out upon a page miss when the free list is
		 empty.
	*/

	//PageRec *freelist; // singly-linked list
    // A hash table that allows one to map a pid to an index into the
    // images buffer (if the page is buffered).
	PageHashMap *pageHash;
	
    // A page replacer implements algorithms for selecting a page to be
    // replaced when the entire buffer is full and a new page has to be
    // brought into the buffer
    //friend class PageReplacer;
    PageReplacer *pageReplacer;

    PagePacker *packer;

public:

    // Constructs a BufferManager for a paged storage container with a
    // memory buffer that holds a given number of pages.
    BufferManager(PagedStorageContainer *s, uint32_t n, PageReplacer *pr=NULL);

    // Destructs the BufferManager.  All dirty pages will be flushed.
    ~BufferManager();
    // Creates a new page in buffer (with unintialized content), pins
    // it, marks it dirty, and returns the handle.
    RC_t allocatePage(PageHandle &ph);

    // Creates a new page with given pid in buffer (with unintialized
    // content), pins it, marks it dirty, and returns the handle.  This
    // method only works if the implementation of PagedStorageContainer
    // supports allocatePageWithPID.
    RC_t allocatePageWithPID(PID_t pid, PageHandle &ph);

    // Disposes a buffered page.  It will be removed from both the
    // buffer and the disk storage.  Dirty bit is ignored.
    RC_t disposePage(PageHandle ph);

    // Reads a page into buffer (if it is not already in), pins it, and
    // returns the handle.
    RC_t readPage(PID_t pid, PageHandle &ph);

    RC_t readOrAllocatePage(PID_t pid, PageHandle &ph);

    // Marks a pinned page as dirty (i.e., modified).
    RC_t markPageDirty(const PageHandle ph);

    // Pins a page.  As long as a page has at least one pin, it cannot
    // be discarded from the buffer.
    RC_t pinPage(const PageHandle ph);

    // Unpins a page.  If a page has no pin left, it can be discarded
    // from the buffer, in which case the handle will become invalid.
    RC_t unpinPage(const PageHandle ph) ;

    // Flushes a pinned page to disk, if it is dirty.  A page's dirty
    // bit is unset after flushing.
    RC_t flushPage(const PageHandle ph) ;

    // Flushes all dirty pages in the buffer to disk.  Pages' dirty bits
    // are unset after flushing.
    RC_t flushAllPages() ;
    
    void print() ;

    void setPagePacker(PagePacker *packer) { this->packer = packer; }

    PID_t getPID(PageHandle ph);
    void *getPageImage(PageHandle ph);
    void *getUnpackedPageImage(PageHandle ph);

private:
	RC_t replacePage(PageRec *&bh);
};

#endif
