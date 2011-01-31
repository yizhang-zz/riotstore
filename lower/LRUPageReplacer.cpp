#include "../common/common.h"
#include "LRUPageReplacer.h"
#include "PageRec.h"
#include "BufferManager.h"

LRUPageReplacer::LRUPageReplacer()
{
	head = NULL;
	tail = NULL;
    count = 0;
}

LRUPageReplacer::~LRUPageReplacer()
{
}

RC_t LRUPageReplacer::selectToReplace(PageRec *&bh)
{
	if (head == NULL)
		return RC_OutOfSpace;
	bh = head;
	remove(bh);
	return RC_OK;
}

int LRUPageReplacer::select(int n, PageRec **pages)
{
    PageRec *p = head;
    int ret = 0;
    while (p && ret < n) {
        pages[ret++] = p;
        p = p->next;
    }
    return ret;
}

void LRUPageReplacer::add(PageRec *bh)
{
	if (tail == NULL) {
		head = tail = bh;
		bh->next = bh->prev = NULL;
	}
	else {
		tail->next = bh;
		bh->prev = tail;
		bh->next = NULL;
		tail = bh;
	}
    count++;
}

void LRUPageReplacer::remove(PageRec *bh)
{
	if (bh->prev == NULL) 
		head = bh->next;
	else
		bh->prev->next = bh->next;

	if (bh->next == NULL) 
		tail = bh->prev;
	else
		bh->next->prev = bh->prev;

	bh->prev = bh->next = NULL;

    count--;
}

void LRUPageReplacer::print()
{
    using namespace std;
    cout<<"free pages: ";
    PageRec *p = head;
    while (p) {
        cout<<*p;
        p = p->next;
    }
    cout<<endl;
}
