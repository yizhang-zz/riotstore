#ifndef PAGE_REPLACER_H
#define PAGE_REPLACER_H

#include "../common/common.h"
#include "BufferManager.h"

//////////////////////////////////////////////////////////////////////
// The interface for a BufferManager helper that decides which
// existing buffered page should be evicted when there is no space
// left.  Note that this class does not implement
// selectToReplace(...), touch(...), and reset(...).  Various
// policies, e.g., LRU, implemented as subclasses of this one, which
// provide actual implementation for these methods.

class PageReplacer {

  friend class BufferManager;

protected:

  // A handle to the buffer manager that this replacer works for
  BufferManager *bufferManager;

  PageReplacer(BufferManager *bm);
  ~PageReplacer();

  // Allows subclasses to access BufferManager's protected/private
  // members that may be useful.
  const bool *getUsedBits() const;
  const bool *getDirtyBits() const;
  const uint32_t *getPinCounts() const;

  // Returns the index (into BufferManager::images) of the recommended
  // page to be replaced when called by BufferManager.  All pages in
  // the buffer should be in use at this point.  Does not perform the
  // actual replacement.
  RC_t selectToReplace(uint32_t &indexOfPageToBeReplaced) const;

  // Called by BufferManager when accessing a page.  indexOfPage is
  // the index into BufferManager::images.
  void touch(uint32_t indexOfPage);

  // Called by BufferManager to reset statistics about a page.
  // indexOfPage is the index into BufferManager::images.
  void reset(uint32_t indexOfPage);
};

#endif
