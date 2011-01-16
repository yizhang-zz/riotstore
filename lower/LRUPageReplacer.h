#ifndef LRU_PAGE_REPLACER_H
#define LRU_PAGE_REPLACER_H

#include "../common/common.h"
#include "PageReplacer.h"

class LRUPageReplacer: public PageReplacer
{
private:
    /*
      Organize all retired, unpinned pages in a list.  The list is
      sorted in increasing last-access time. Head of the list is the
      least recently accessed.
	*/
	PageRec *head, *tail;
    size_t count;
public:
    LRUPageReplacer();
    ~LRUPageReplacer();
    RC_t selectToReplace(PageRec *&bh);
    void add(PageRec *bh);
    void remove(PageRec *bh);
    void print();
    size_t size() const { return count; }
};

#endif
