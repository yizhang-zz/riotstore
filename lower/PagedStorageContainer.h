#ifndef PAGED_STORAGE_CONTAINER_H
#define PAGED_STORAGE_CONTAINER_H

#include "../common/common.h"
#include "PageRec.h"

/**
 * Provides a paged disk-based storage container, which allows allocation of
 * new pages and disposal, reading, and writing of pages by pid.
 */

class PagedStorageContainer {

public:

	const static int CREATE = 0x1;

#ifdef PROFILING
    static int readCount;
    static int writeCount;
    static double accessTime;

    static void resetPerfCounts()
    {
        readCount = writeCount = 0;
        accessTime = 0.0;
    }
#endif

	virtual ~PagedStorageContainer() {}

	/**
     * Creates a new page in the storage and returns the pid of the new
	 * page.  Space of a previously disposed page may be reused.  The
	 * content remains unintialized until the first write.
     */
	virtual RC_t allocatePage(PID_t &pid) = 0;

	/**
     * Creates a new page with given pid in the storage.  The content
     * remains unintialized until the first write.  This request may or
     * may not be supported by an implementation.
     */
	virtual RC_t allocatePageWithPID(PID_t pid) = 0;

	/**
     * Disposes the page with given pid from the file (so that its space
     * could be allocated later).
     */
	virtual RC_t disposePage(PID_t pid) = 0;

	/**
     * Reads a page into memory.  The pid and the memory location for
	 * the image are given in the page handle.
     */
	virtual RC_t readPage(PageRec *) = 0;

	/**
     * Writes a page to the storage.  The pid and the memory location
	 * for the image are given in the page handle.  The page must have
	 * been allocated before.
     */
	virtual RC_t writePage(PageRec *) = 0;

};
#endif
