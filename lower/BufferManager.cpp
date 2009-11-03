#include "BufferManager.h"
#include <unordered_map>
#include <string.h>
template <typename T>
BufferManager<T>::BufferManager(PagedStorageContainer *s, uint32_t numSlots)
{
	storage = s;
	images = new PageImage[numSlots];
	// init free list
	headers = new BufferHeader[numSlots];
	freelist = headers;
	for (int i=0; i<numSlots-1; i++) {
		freelist[i].next = freelist+(i+1);
		//freelist[i+1].prev = freelist+i;
		freelist[i].pid = 0;
		freelist[i].dirty = false;
		freelist[i].pinCount = 0;
		freelist[i].image = images+i;
	}
	//freelist[0].prev = NULL;
	freelist[numSlots-1].next = NULL;
	
	//usedBits = new bool[numSlots];
	//dirtyBits = new bool[numSlots];
	//pinCounts = new uint32_t[numSlots];
	//pids = new PID_t[numSlots];
	pageHash = new PageHashMap();

	// initialization
	//memset(usedBits, 0, numSlots*sizeof(bool));
	//memset(dirtyBits, 0, numSlots*sizeof(bool));
	//memset(pinCounts, 0, numSlots*sizeof(uint32_t));
	//memset(pids, 0, numSlots*sizeof(pids));

	pageReplacer = new T(this);
}

template<typename T>
BufferManager<T>::~BufferManager()
{
	delete[] images;
	delete[] headers;
	//delete[] dirtyBits;
	//delete[] pinCounts;
	//delete[] pids;
	delete pageHash;
	delete pageReplacer;
}

template<typename T>
RC_t BufferManager<T>::allocatePage(PageHandle &ph)
{
	BufferHeader *bh;
	if ( storage->allocatePage(ph.pid) != RC_SUCCESS) {
		printf("Physical storage cannot allocate page.\n");
		return RC_FAILURE;
	}
	// check free list first
	if (freelist != NULL) {
		bh = freelist;
		ph.image = bh->image;
		freelist = freelist->next;
	}	
	else {
		if (replacePage(&bh) != RC_SUCCESS) {
			return RC_FAILURE;
		}
		ph.image = bh->image;
	}	
	bh->pinCount = 1;
	bh->dirty = 1;
	bh->pid = ph.pid;
	pageHash->insert(PageHashMap::value_type(ph.pid, bh));
	return RC_SUCCESS;
}

template<typename T>
RC_t BufferManager<T>::allocatePageWithPID(PID_t pid, PageHandle &ph)
{
	BufferHeader *bh;
	if (storage->allocatePageWithPID(pid) != RC_SUCCESS) {
		printf("Physical storage cannot allocate page.\n");
		return RC_FAILURE;
	}
	ph.pid = pid;
	// check free list first
	if (freelist != NULL) {
		bh = freelist;
		ph.image = bh->image;
		freelist = freelist->next;
	}	
	else {
		if (replacePage(&bh) != RC_SUCCESS) {
			return RC_FAILURE;
		}
		ph.image = bh->image;
	}	
	bh->pinCount = 1;
	bh->dirty = 1;
	bh->pid = ph.pid;
	pageHash->insert(PageHashMap::value_type(ph.pid, bh));
	return RC_SUCCESS;
}

template<class T>
RC_t BufferManager<T>::disposePage(const PageHandle &ph)
{
	if (storage->disposePage(ph.pid) != RC_SUCCESS) {
		printf("Physical storage cannot dispose page.\n");
		return RC_FAILURE;
	}
	PageHashMap::iterator it = pageHash->find(ph.pid);
	BufferHeader *bh = it->second;
	// the page should not be pinned
	assert(bh->pinCount == 0);
	if (it != pageHash->end()) {
		pageHash->erase(it);
	}
	// move the page to free list
	pageReplacer->remove(bh);
	InitBufferHeader(bh);
	bh->next = freelist;	
	freelist = bh;
	return RC_SUCCESS;	
}

template<class T>
RC_t BufferManager<T>::readPage(PageHandle &ph)
{
	// first check if already buffered
	PageHashMap::iterator it = pageHash->find(ph.pid);
	if (it != pageHash->end()) {
		BufferHeader *bh = it->second;
		bh->pinCount++;
		ph.image = bh->image;
		if (bh->pinCount == 1)
			pageReplacer->remove(bh);
	}
	else {
		// check free list first
		BufferHeader *bh;
		if (freelist != NULL) {
			bh = freelist;
			ph.image = bh->image;
			freelist = freelist->next;
		}
		else {
			if (replacePage(&bh) != RC_SUCCESS) {
				return RC_FAILURE;
			}
			ph.image = bh->image;
		}
		InitBufferHeader(bh);
		bh->pid = ph.pid;
		bh->pinCount = 1;
		
		if (storage->readPage(ph) != RC_SUCCESS) {
			printf("Physical storage cannot read page.\n");
			return RC_FAILURE;
		}
	}
	return RC_SUCCESS;
}

template<class T>
RC_t BufferManager<T>::replacePage(BufferHeader **bh)
{
	if (pageReplacer->selectToReplace(bh) != RC_SUCCESS) {
		printf("Out of memory: cannot allocate page in buffer.\n");
		return RC_FAILURE;
	}
	if ((*bh)->dirty) {
		PageHandle ph;
		ph.pid = (*bh)->pid;
		ph.image = (*bh)->image;
		if (storage->writePage(ph) != RC_SUCCESS) {
			printf("Physical storage cannot write page.\n");
			return RC_FAILURE;
		}
		(*bh)->dirty = 0;
	}
	return RC_SUCCESS;
}

template<class T>
RC_t BufferManager<T>::markPageDirty(const PageHandle &ph)
{
	PageHashMap::iterator it = pageHash->find(ph.pid);
	assert(it != pageHash->end());
	BufferHeader *bh = it->second;
	bh->dirty = 1;
	return RC_SUCCESS;
}

template<class T>
RC_t BufferManager<T>::pinPage(const PageHandle &ph)
{
	// the page must have already been pinned (when it was created)	
	PageHashMap::iterator it = pageHash->find(ph.pid);
	assert(it != pageHash->end());
	BufferHeader *bh = it->second;
	bh->pinCount++;
	return RC_SUCCESS;
}

template<class T>
RC_t BufferManager<T>::unpinPage(const PageHandle &ph)
{
	PageHashMap::iterator it = pageHash->find(ph.pid);
	assert(it != pageHash->end());
	BufferHeader *bh = it->second;
	bh->pinCount--;
	if (bh->pinCount == 0) {
		// Access to this page has all terminated;
		// Let PageReplacer manager this page
		pageReplacer->add(bh);
	}
	return RC_SUCCESS;
}

template<class T>
RC_t BufferManager<T>::flushPage(const PageHandle &ph)
{
	PageHashMap::iterator it = pageHash->find(ph.pid);
	assert(it != pageHash->end());
	BufferHeader *bh = it->second;
	if (bh->dirty) {
		if (storage->writePage(ph) != RC_SUCCESS) {
			printf("Physical storage cannot write page.\n");
			return RC_FAILURE;
		}
		bh->dirty = 0;
	}
	return RC_SUCCESS;
}

template<class T>
RC_t BufferManager<T>::flushAllPages()
{
	bool good = true;
	for (PageHashMap::iterator it = pageHash->begin();
			 it != pageHash->end();
			 it++) {
		BufferHeader *bh = it->second;
		PageHandle ph;
		ph.pid = bh->pid;
		ph.image = bh->image;
		if (bh->dirty) {
			if (storage->writePage(ph) != RC_SUCCESS) {
				printf("Physical storage cannot write page.\n");
				good = false;
			}
			bh->dirty = 0;
		}
	}
	return good ? RC_SUCCESS : RC_FAILURE;
}

