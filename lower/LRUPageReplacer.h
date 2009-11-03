#ifndef LRU_PAGE_REPLACER_H
#define LRU_PAGE_REPLACER_H

#include "../common/common.h"
#include "PageReplacer.h"

struct BufferHeader;

class LRUPageReplacer: public PageReplacer {
protected:
  /*
		Organize all retired, unpinned pages in a list.  The list is
		sorted in increasing last-access time. Head of the list is the
		least recently accessed.
	*/
	BufferHeader *head, *tail;
protected:
  LRUPageReplacer(/*BufferManager &bufferManager*/);
  ~LRUPageReplacer();
  RC_t selectToReplace(BufferHeader **bh);
  void add(BufferHeader *bh);
  void remove(BufferHeader *bh);
};

#endif
