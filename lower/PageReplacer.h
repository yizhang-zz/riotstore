#ifndef PAGE_REPLACER_H
#define PAGE_REPLACER_H

#include "../common/common.h"
//#include "BufferManager.h"

//class BufferManager;
struct BufferHeader;
//////////////////////////////////////////////////////////////////////
// The interface for a BufferManager helper that decides which
// existing buffered page should be evicted when there is no space
// left.  Note that this class does not implement
// selectToReplace(...), touch(...), and reset(...).  Various
// policies, e.g., LRU, implemented as subclasses of this one, which
// provide actual implementation for these methods.

class PageReplacer {

//  friend class BufferManager;

public:

  // A handle to the buffer manager that this replacer works for
  // BufferManager *bufferManager;

  PageReplacer() {}
  virtual ~PageReplacer() {}

  // Allows subclasses to access BufferManager's protected/private
  // members that may be useful.
  /* const bool *getUsedBits() const; */
  /* const bool *getDirtyBits() const; */
  /* const uint32_t *getPinCounts() const; */

  // Returns the index (into BufferManager::images) of the recommended
  // page to be replaced when called by BufferManager.  All pages in
  // the buffer should be in use at this point.  Does not perform the
  // actual replacement.
  RC_t selectToReplace(BufferHeader **bh);

  // Called by BufferManager when unpinning a page.  indexOfPage is
  // the index into BufferManager::images.
  // When a page is accessed, it must have been pinned already. On the
  // other hand, if a page is to be unpinned, it must have been
  // accessed at least once. So we can always touch it when it is
  // unpinned.
  void add(BufferHeader *bh);
	void remove(BufferHeader *bh);

  // Called by BufferManager to reset statistics about a page.
  // indexOfPage is the index into BufferManager::images.
  //void reset(uint32_t indexOfPage);
};

#endif
