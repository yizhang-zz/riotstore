#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "../common/common.h"
#include "PagedStorageContainer.h"

// forward declaration
class PageReplacer;

//////////////////////////////////////////////////////////////////////
// Provides memory-buffered access to a PagedStorageContainer.  This
// is the interface that data structures such as BTree and
// DirectlyMappedArray should use.  BTree would call allocatePage and
// disposePage, but never allocatePageWithPid.  DirectlyMappedArray
// would call allocatePageWithPid, but never allocatePage or
// disposePage.

class BufferManager {

private:

  // The underlying storage.
  PagedStorageContainer *storage;

  // Maximum number of pages the buffer can hold.
  uint32_t numSlots;

  // The buffer memory.  It is implemented as an array of size
  // numSlots.
  PageImage *images;

  // An array of bits indicating whether the corresponding slots in
  // the images buffer are in use.
  bool *usedBits;

  // An array of bits indicating whether the corresponding pages are
  // dirty (valid only for slots in use).
  bool *dirtyBits;

  // An array of counts indicating the number of pins for each page
  // (valid only for slots in use).
  uint32_t *pinCounts;

  // An array of pids for the corresponding pages in the images buffer
  // (valid only for slots in use).
  pgid_t *pids;

  // A hash table that allows one to map a pid to an index into the
  // images buffer (if the page is buffered).
  HashTable *pidToIndexIntoPages;

  // A page replacer implements algorithms for selecting a page to be
  // replaced when the entire buffer is full and a new page has to be
  // brought into the buffer
  friend class PageReplacer;
  PageReplacer *pageReplacer;

public:

  // Constructs a BufferManager for a paged storage container with a
  // memory buffer that holds a given number of pages.
  BufferManager(PagedStorageContainer *storage, uint32_t numSlots);

  // Destructs the BufferManager.  All dirty pages will be flushed.
  ~BufferManager();

  // Creates a new page in buffer (with unintialized content), pins
  // it, marks it dirty, and returns the handle.
  rc_t allocatePage(PageHandle &ph);

  // Creates a new page with given pid in buffer (with unintialized
  // content), pins it, marks it dirty, and returns the handle.  This
  // method only works if the implementation of PagedStorageContainer
  // supports allocatePageWithPid.
  rc_t allocatePageWithPid(pgid_t pid, PageHandle &ph);

  // Disposes a buffered page.  It will be removed from both the
  // buffer and the disk storage.  Dirty bit is ignored.
  rc_t disposePage(const PageHandle &ph);

  // Reads a page into buffer (if it is not already in), pins it, and
  // returns the handle.
  rc_t readPage(pgid_t pid, PageHandle &ph);

  // Marks a pinned page as dirty (i.e., modified).
  rc_t markPageDirty(const PageHandle &ph);

  // Pins a page.  As long as a page has at least one pin, it cannot
  // be discarded from the buffer.
  rc_t pinPage(const PageHandle &ph);

  // Unpins a page.  If a page has no pin left, it can be discarded
  // from the buffer, in which case the handle will become invalid.
  rc_t unpinPage(const PageHandle &ph);

  // Flushes a pinned page to disk, if it is dirty.  A page's dirty
  // bit is unset after flushing.
  rc_t flushPage(const PageHandle &ph);

  // Flushes all dirty pages in the buffer to disk.  Pages' dirty bits
  // are unset after flushing.
  rc_t flushAllPages();

};

#endif
