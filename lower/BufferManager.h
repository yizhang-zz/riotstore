#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include <stdio.h>
#include <assert.h>
#include <tr1/unordered_map>
#include "../common/common.h"
#include "PagedStorageContainer.h"

// forward declaration
class PageReplacer;
class LRUPageReplacer;

struct BufferHeader {
	BufferHeader *prev;
	BufferHeader *next;
	PID_t pid;
	bool dirty;
	uint32_t pinCount;
	PageImage *image;
};
 
inline void InitBufferHeader(BufferHeader *bh)
{
	bh->prev = bh->next = NULL;
	bh->pid = 0;
	bh->dirty = 0;
	bh->pinCount = 0;
}

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

template <typename T = LRUPageReplacer>
class BufferManager {

private:

  typedef std::tr1::unordered_map<PID_t, BufferHeader*> PageHashMap;

  // The underlying storage.
  PagedStorageContainer *storage;

  // Maximum number of pages the buffer can hold.
  uint32_t numSlots;

  // The buffer memory.  It is implemented as an array of size
  // numSlots.
  PageImage *images;

	// A header for each image
	BufferHeader *headers;

  // An array of bits indicating whether the corresponding slots in
  // the images buffer are in use.
  //bool *usedBits;

  // An array of bits indicating whether the corresponding pages are
  // dirty (valid only for slots in use).
  //bool *dirtyBits;

  // An array of counts indicating the number of pins for each page
  // (valid only for slots in use).
  //uint32_t *pinCounts;

  // An array of pids for the corresponding pages in the images buffer
  // (valid only for slots in use).
  //PID_t *pids;

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

	BufferHeader *freelist; // singly-linked list
  // A hash table that allows one to map a pid to an index into the
  // images buffer (if the page is buffered).
	PageHashMap *pageHash;
	
  // A page replacer implements algorithms for selecting a page to be
  // replaced when the entire buffer is full and a new page has to be
  // brought into the buffer
  //friend class PageReplacer;
  T *pageReplacer;

public:

  // Constructs a BufferManager for a paged storage container with a
  // memory buffer that holds a given number of pages.
  BufferManager(PagedStorageContainer *s, uint32_t n) {
		numSlots = n;
		storage = s;
		images = new PageImage[numSlots];
		// init free list
		headers = new BufferHeader[numSlots];
		freelist = headers;
		for (int i=0; i<numSlots; i++) {
			freelist[i].next = freelist+(i+1);
			//freelist[i+1].prev = freelist+i;
			freelist[i].pid = 0;
			freelist[i].dirty = false;
			freelist[i].pinCount = 0;
			freelist[i].image = images+i;
		}
		//freelist[0].prev = NULL;
		freelist[numSlots-1].next = NULL;
	
		//usedBits = new bool[numSlots];
		//dirtyBits = new bool[numSlots];
		//pinCounts = new uint32_t[numSlots];
		//pids = new PID_t[numSlots];
		pageHash = new PageHashMap();

		// initialization
		//memset(usedBits, 0, numSlots*sizeof(bool));
		//memset(dirtyBits, 0, numSlots*sizeof(bool));
		//memset(pinCounts, 0, numSlots*sizeof(uint32_t));
		//memset(pids, 0, numSlots*sizeof(pids));

		pageReplacer = new T();
	}

  // Destructs the BufferManager.  All dirty pages will be flushed.
  ~BufferManager() {
		flushAllPages();
		delete[] images;
		delete[] headers;
		delete pageHash;
		delete pageReplacer;
	}

  // Creates a new page in buffer (with unintialized content), pins
  // it, marks it dirty, and returns the handle.
  RC_t allocatePage(PageHandle &ph) {
		BufferHeader *bh;
		if ( storage->allocatePage(ph.pid) != RC_SUCCESS) {
			printf("Physical storage cannot allocate page.\n");
			return RC_FAILURE;
		}
		// check free list first
		if (freelist != NULL) {
			bh = freelist;
			ph.image = bh->image;
			freelist = freelist->next;
		}	
		else {
			if (replacePage(&bh) != RC_SUCCESS) {
				return RC_FAILURE;
			}
			ph.image = bh->image;
		}	
		bh->pinCount = 1;
		bh->dirty = 1;
		bh->pid = ph.pid;
		pageHash->insert(PageHashMap::value_type(ph.pid, bh));
		return RC_SUCCESS;
	}

  // Creates a new page with given pid in buffer (with unintialized
  // content), pins it, marks it dirty, and returns the handle.  This
  // method only works if the implementation of PagedStorageContainer
  // supports allocatePageWithPID.
  RC_t allocatePageWithPID(PID_t pid, PageHandle &ph) {
		BufferHeader *bh;
		if (storage->allocatePageWithPID(pid) != RC_SUCCESS) {
			printf("Physical storage cannot allocate page.\n");
			return RC_FAILURE;
		}
		ph.pid = pid;
		// check free list first
		if (freelist != NULL) {
			bh = freelist;
			ph.image = bh->image;
			freelist = freelist->next;
		}	
		else {
			if (replacePage(&bh) != RC_SUCCESS) {
				return RC_FAILURE;
			}
			ph.image = bh->image;
		}	
		bh->pinCount = 1;
		bh->dirty = 1;
		bh->pid = ph.pid;
		pageHash->insert(PageHashMap::value_type(ph.pid, bh));
		return RC_SUCCESS;
	}

  // Disposes a buffered page.  It will be removed from both the
  // buffer and the disk storage.  Dirty bit is ignored.
  RC_t disposePage(const PageHandle &ph) {
		if (storage->disposePage(ph.pid) != RC_SUCCESS) {
			printf("Physical storage cannot dispose page.\n");
			return RC_FAILURE;
		}
		PageHashMap::iterator it = pageHash->find(ph.pid);
		BufferHeader *bh = it->second;
		// the page should not be pinned
		assert(bh->pinCount == 0);
		if (it != pageHash->end()) {
			pageHash->erase(it);
		}
		// move the page to free list
		pageReplacer->remove(bh);
		InitBufferHeader(bh);
		bh->next = freelist;	
		freelist = bh;
		return RC_SUCCESS;
	}

  // Reads a page into buffer (if it is not already in), pins it, and
  // returns the handle.
  RC_t readPage(PageHandle &ph) {
		// first check if already buffered
		PageHashMap::iterator it = pageHash->find(ph.pid);
		if (it != pageHash->end()) {
			BufferHeader *bh = it->second;
			bh->pinCount++;
			ph.image = bh->image;
			if (bh->pinCount == 1)
				pageReplacer->remove(bh);
		}
		else {
			// check free list first
			BufferHeader *bh;
			if (freelist != NULL) {
				bh = freelist;
				ph.image = bh->image;
				freelist = freelist->next;
			}
			else {
				if (replacePage(&bh) != RC_SUCCESS) {
					return RC_FAILURE;
				}
				ph.image = bh->image;
			}
			InitBufferHeader(bh);
			bh->pid = ph.pid;
			bh->pinCount = 1;
			pageHash->insert(PageHashMap::value_type(ph.pid, bh));
			
			if (storage->readPage(ph) != RC_SUCCESS) {
				printf("Physical storage cannot read page.\n");
				return RC_FAILURE;
			}
		}
		return RC_SUCCESS;
	}

  // Marks a pinned page as dirty (i.e., modified).
  RC_t markPageDirty(const PageHandle &ph) {
		PageHashMap::iterator it = pageHash->find(ph.pid);
		assert(it != pageHash->end());
		BufferHeader *bh = it->second;
		bh->dirty = 1;
		return RC_SUCCESS;
	}

  // Pins a page.  As long as a page has at least one pin, it cannot
  // be discarded from the buffer.
  RC_t pinPage(const PageHandle &ph) {
		// the page must have already been pinned (when it was created)	
		PageHashMap::iterator it = pageHash->find(ph.pid);
		assert(it != pageHash->end());
		BufferHeader *bh = it->second;
		bh->pinCount++;
		return RC_SUCCESS;
	}

  // Unpins a page.  If a page has no pin left, it can be discarded
  // from the buffer, in which case the handle will become invalid.
  RC_t unpinPage(const PageHandle &ph) {
		PageHashMap::iterator it = pageHash->find(ph.pid);
		assert(it != pageHash->end());
		BufferHeader *bh = it->second;
		bh->pinCount--;
		if (bh->pinCount == 0) {
			// Access to this page has all terminated;
			// Let PageReplacer manager this page
			pageReplacer->add(bh);
		}
		return RC_SUCCESS;
	}

  // Flushes a pinned page to disk, if it is dirty.  A page's dirty
  // bit is unset after flushing.
  RC_t flushPage(const PageHandle &ph) {
		PageHashMap::iterator it = pageHash->find(ph.pid);
		assert(it != pageHash->end());
		BufferHeader *bh = it->second;
		if (bh->dirty) {
			if (storage->writePage(ph) != RC_SUCCESS) {
				printf("Physical storage cannot write page.\n");
				return RC_FAILURE;
			}
			bh->dirty = 0;
		}
		return RC_SUCCESS;
	}

  // Flushes all dirty pages in the buffer to disk.  Pages' dirty bits
  // are unset after flushing.
  RC_t flushAllPages() {
		bool good = true;
		for (PageHashMap::iterator it = pageHash->begin();
				 it != pageHash->end();
				 it++) {
			BufferHeader *bh = it->second;
			PageHandle ph;
			ph.pid = bh->pid;
			ph.image = bh->image;
			if (bh->dirty) {
				if (storage->writePage(ph) != RC_SUCCESS) {
					printf("Physical storage cannot write page.\n");
					good = false;
				}
				bh->dirty = 0;
			}
		}
		return good ? RC_SUCCESS : RC_FAILURE;
	}

private:
	RC_t replacePage(BufferHeader **bh)
	{
		if (pageReplacer->selectToReplace(bh) != RC_SUCCESS) {
			printf("Out of memory: cannot allocate page in buffer.\n");
			return RC_FAILURE;
		}
		if ((*bh)->dirty) {
			PageHandle ph;
			ph.pid = (*bh)->pid;
			ph.image = (*bh)->image;
			if (storage->writePage(ph) != RC_SUCCESS) {
				printf("Physical storage cannot write page.\n");
				return RC_FAILURE;
			}
			(*bh)->dirty = 0;
		}
		return RC_SUCCESS;
	}

};

#endif
