#ifndef PAGE_REPLACER_H
#define PAGE_REPLACER_H

#include "../common/common.h"
#include "BufferManager.h"

//////////////////////////////////////////////////////////////////////
// When the buffer manager has no space for a new page, this class is
// used to decide which existing buffered page should be evicted.
// Various algorithms can be implemented, e.g., LRU.

class PageReplacer {

private:

  // A handle to the buffer manager that this replacer works for
  BufferManager *bufferManager;

  // TODO : some data structure in subclass will be needed to implement the
  // page replacement policy like LRU or CLOCK.

public:

  PageReplacer(BufferManager *bm);

  // Remember to destruct any class-specific data structures.
  virtual ~PageReplacer();

  // Called by buffer manager when deciding which page to be replaced.
  // Does not perform the actual replacement.
  virtual rc_t replace(uint32_t &indexOfPageToBeReplaced);

  // Called by buffer manager when a page is accessed.
  virtual rc_t accessed(uint32_t indexOfPage);

  // Called by buffer manager when a page is allocated.
  virtual rc_t allocated(uint32_t indexOfPage);
};

#endif
