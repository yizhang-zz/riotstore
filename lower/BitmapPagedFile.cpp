#include <unistd.h>
#include <malloc.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "BitmapPagedFile.h"
#include "PageRec.h"

int BitmapPagedFile::readCount = 0;
int BitmapPagedFile::writeCount = 0;

/** Creates a BitmapPagedFile over a disk file of a given name. If flag has
 * F_CREATE set, then a new file is created; otherwise the file is assumed
 * to exist. It is the caller's responsibility to ensure the existence of
 * the file.
 *
 * Direct I/O is used to bypass OS' buffering.
 *
 * Bit ordering in header: 7,6,5,4,3,2,1,0|7,6,5,4,3,2,1,0|7,6....
 * a bit expressed as (k, n)tuple where the bit is contained within the kth
 * byte in header and is the nth most significant bit (0 being least
 * significant) of that byte.
 * define pid = 8*k + n
 * when a new page is needed, the smallest available pid is assigned.
 * numContentPages <= 8*PAGE_SIZE --> maximum number of pages in file container
 * maximum allocated pid < numContentPages
 * pid to bitmask mapping:
 * 7,6,5,4,3,2,1,0|15,14,13,12,11,10,9,8|23,22,...
 * header is allocated 1 byte at a time. (when first 8 bits are used and need to
 * allocate another, next 8 bits are cleared).
 */

BitmapPagedFile::BitmapPagedFile(const char *pathname, int flag) {
    header = (Byte_t*)memalign(PAGE_SIZE, PAGE_SIZE);
    readCount = 0;
	// create new file
	if (flag & F_CREATE) {
		fd = open_direct(pathname, O_RDWR|O_CREAT|O_TRUNC);
        if (fd < 0) {
            fprintf(stderr, "error %d\n", errno);
            exit(1);
        }
		assert(fd >= 0);
        numContentPages = 0;
        memset(header, 0, PAGE_SIZE);
        int c = write(fd, header, PAGE_SIZE);
        if (c <= 0) {
            fprintf(stderr, "written %d\n, %p\n", c, header);
        }
    }
	// file exists
    else { // at least header here
		fd = open_direct(pathname, O_RDWR);
        if (fd < 0) {
            fprintf(stderr, "error %d\n", errno);
            exit(1);
        }

		assert(fd >= 0);
		struct stat s;
		fstat(fd, &s);
        numContentPages = s.st_size/PAGE_SIZE - 1;
		lseek(fd, 0, SEEK_SET);
        read(fd, header, PAGE_SIZE);
    }
}

// Remember to write the header back!
BitmapPagedFile::~BitmapPagedFile() {
    // make sure number of pages in file is consistent with numContentPages
    Byte_t *filler = header;
	struct stat s;
	fstat(fd, &s);
    if(1 + numContentPages > s.st_size/PAGE_SIZE) {
        lseek(fd, (numContentPages)*PAGE_SIZE, SEEK_SET);
        write(fd, filler, PAGE_SIZE);
    }
    // write header
	lseek(fd, 0, SEEK_SET);
    int c = write(fd, header, PAGE_SIZE);
    close(fd);
    free(header);
}

/* only mark bit in header as allocated, defer writing until later */
RC_t BitmapPagedFile::allocatePage(PID_t &pid) {
    // first check if next bit beyond currently allocated ones is available
    if(numContentPages < 8*PAGE_SIZE) { // still unused bits in header
        pid = numContentPages;
        allocate(pid);
        numContentPages++;
        return RC_OK;
    }

    // check for empty slot in allocated header space
    for(unsigned k = 0; k < numContentPages; k+=8) { // check 1 byte at a time
        if((header[k/8] & 255) != 255) { // found empty slot in header[k/8]
            Byte_t target = header[k/8];
            Byte_t mask = 1;
			int i = 0;
			while (target & mask) {
				mask <<= 1;
				i++;
			}
			pid = k + i;
			allocate(pid);
			return RC_OK;
        }
    }
    return RC_OutOfSpace; // header filled
}

/* tries to allocate new page with input pid. if pid is available, bitmask flag
 * is made. otherwise returns failure. The page is not only allocated in memory
 * and not written until later call
 */
RC_t BitmapPagedFile::allocatePageWithPID(PID_t pid) {
    if(pid >= 8*PAGE_SIZE) { // pid outside valid range
        return RC_OutOfRange;
    }

    // if pid is within range of current allocated pages
    if(pid < numContentPages) {
        if(isAllocated(pid)) { // pid already allocated
            return RC_AlreadyAllocated;
        }
        allocate(pid);
        return RC_OK;
    }

    // pid not in current page ranges, but within maximum page bounds
    /*int bytes = (pid - numContentPages)/8;
      for(; numContentPages < pid; numContentPages++) { // check this
      if(numContentPages%8 == 0) {
      header[numContentPages/8] = 0;
      }
      }
     */
    numContentPages = pid + 1;
    allocate(pid);
    return RC_OK;
}

RC_t BitmapPagedFile::disposePage(PID_t pid) {
    if(pid >= numContentPages) {
        return RC_OutOfRange; /* pid not within allocated pid range, nothing is done */
    }

    if(isAllocated(pid)) {
        deallocate(pid); /* deallocates mapped bit in header, defers actual
                            physical disposal of data until later */
        return RC_OK;
    }
    return RC_NotAllocated;
}

RC_t BitmapPagedFile::readPage(PageHandle ph) {
    // pid is not in usable region yet or is unallocated
    PageRec *rec = (PageRec*)ph;
    PID_t pid = rec->pid;
    if(pid >= numContentPages)
        return RC_OutOfRange;
    if(!isAllocated(pid)) 
        return RC_NotAllocated;

    lseek(fd, (1 + pid)*PAGE_SIZE, SEEK_SET); // +1 for header page
    read(fd, rec->image, PAGE_SIZE);
    readCount++;
    return RC_OK;
}

// writes ph to file if pid has been allocated
RC_t BitmapPagedFile::writePage(PageHandle ph) {
    PageRec *rec = (PageRec*) ph;
    PID_t pid = rec->pid;
    // pid is not in usable region yet or is unallocated
    if(pid >= numContentPages)
        return RC_OutOfRange;
    if(!isAllocated(pid)) 
        return RC_NotAllocated;

    /*
    fseek(file, 0, SEEK_END);
    int numContentPagesOnFile = ftell(file)/PAGE_SIZE - 1;
    if(ph.pid >= numContentPagesOnFile) { // need to add page padding
        Byte_t *filler = header; // use header as filler page
        // leave padding to be 1 less than total pages on file, to add ph at end
        int numPagePads = ph.pid - numContentPagesOnFile;
        for(int k = 0; k < numPagePads; k++) {
            fwrite(filler, PAGE_SIZE, 1, file);
        }
    }
    */
    lseek(fd, (1 + pid)*PAGE_SIZE, SEEK_SET); // +1 for header page
    write(fd, rec->image, PAGE_SIZE);
    writeCount++;
    return RC_OK;
}
