#include "../common/common.h"
#include "LRUPageReplacer.h"
#include "PageRec.h"
#include "BufferManager.h"

LRUPageReplacer::LRUPageReplacer()
{
    head.reset();
    head.next = head.prev = &head;
    count = 0;
}

LRUPageReplacer::~LRUPageReplacer()
{
}

// Head of the list is least recently used
RC_t LRUPageReplacer::selectToReplace(PageRec *&bh)
{
	if (head.next == &head)
		return RC_OutOfSpace;
	bh = head.next;
	remove(bh);
	return RC_OK;
}

void LRUPageReplacer::add(PageRec *bh)
{
    head.prev->next = bh;
    bh->prev = head.prev;
    bh->next = &head;
    head.prev = bh;
    count++;
}

void LRUPageReplacer::addback(PageRec *bh)
{
    bh->next = head.next;
    head.next->prev = bh;
    bh->prev = &head;
    head.next = bh;
}

void LRUPageReplacer::remove(PageRec *bh)
{
    bh->prev->next = bh->next;
    bh->next->prev = bh->prev;
	bh->prev = bh->next = NULL;
    count--;
}

void LRUPageReplacer::print()
{
    using namespace std;
    cout<<"free pages: ";
    PageRec *p = head.next;
    while (p != &head) {
        cout<<*p;
        p = p->next;
   }
    cout<<endl;
}
