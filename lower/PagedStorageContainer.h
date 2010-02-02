#ifndef PAGED_STORAGE_CONTAINER_H
#define PAGED_STORAGE_CONTAINER_H

#include "../common/common.h"

//////////////////////////////////////////////////////////////////////
// Provides a paged disk-based storage container, which allows
// allocation of new pages and disposal, reading, and writing of pages
// by pid.

class PagedStorageContainer {

public:

	const static int F_CREATE = 01;
	const static int F_NO_CREATE = 02;

	virtual ~PagedStorageContainer() {}

	// Creates a new page in the storage and returns the pid of the new
	// page.  Space of a previously disposed page may be reused.  The
	// content remains unintialized until the first write.
	virtual RC_t allocatePage(PID_t &pid) = 0;

	// Creates a new page with given pid in the storage.  The content
	// remains unintialized until the first write.  This request may or
	// may not be supported by an implementation.
	virtual RC_t allocatePageWithPID(PID_t pid) = 0;

	// Disposes the page with given pid from the file (so that its space
	// could be allocated later).
	virtual RC_t disposePage(PID_t pid) = 0;

	// Reads a page into memory.  The pid and the memory location for
	// the image are given in the page handle.
	virtual RC_t readPage(PageHandle &ph) = 0;

	// Writes a page to the storage.  The pid and the memory location
	// for the image are given in the page handle.  The page must have
	// been allocated before.
	virtual RC_t writePage(const PageHandle &ph) = 0;

};
#endif
