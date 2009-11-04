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

RC_t LRUPageReplacer::selectToReplace(BufferHeader **bh)
{
	if (head == NULL)
		return RC_FAILURE;
	*bh = head;
	head = head->next;
	if (head != NULL)
		head->prev = NULL;
	(*bh)->prev = (*bh)->next = NULL;
	return RC_SUCCESS;
}

void LRUPageReplacer::add(BufferHeader *bh)
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

void LRUPageReplacer::remove(BufferHeader *bh)
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
