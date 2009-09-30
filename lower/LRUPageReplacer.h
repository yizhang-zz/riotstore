#ifndef LRU_PAGE_REPLACER_H
#define LRU_PAGE_REPLACER_H

#include "../common/common.h"
#include "PageReplacer.h"

class LRUPageReplacer: public PageReplacer {
protected:
  // TODO: Some data structure (likely an array holding statistics for
  // pages in BufferManager::images) will be needed here.
protected:
  LRUPageReplacer(BufferManager &bufferManager);
  ~LRUPageReplacer();
  RC_t selectToReplace(uint32_t &indexOfPageToBeReplaced) const;
  void touch(uint32_t indexOfPage);
  void reset(uint32_t indexOfPage);
};

#endif
