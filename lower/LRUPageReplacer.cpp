#include "../common/common.h"
#include "LRUPageReplacer.h"
#include "BufferManager.h"

LRUPageReplacer::LRUPageReplacer()
{
	head = NULL;
	tail = NULL;
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
}
