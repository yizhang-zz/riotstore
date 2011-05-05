#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "BitmapPagedFile.h"
#include "PageRec.h"

#define _XOPEN_SOURCE 600
#include <fcntl.h>

#ifdef PROFILING
int PagedStorageContainer::readCount = 0;
int PagedStorageContainer::writeCount = 0;
double PagedStorageContainer::accessTime = 0.0;
#endif

const size_t BitmapPagedFile::NUM_HEADER_PAGES = STORAGE_METADATA_PAGES;
const size_t BitmapPagedFile::NUM_BITS_PER_PAGE = PAGE_SIZE * 8;
const size_t BitmapPagedFile::HEADER_SIZE = PAGE_SIZE * BitmapPagedFile::NUM_HEADER_PAGES;
const size_t BitmapPagedFile::NUM_BITS_HEADER = BitmapPagedFile::HEADER_SIZE * 8;

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
	header = (u32*)allocPageImage(NUM_HEADER_PAGES);
	// create new file
	if (flag & CREATE) {
		fd = riot_open(pathname, O_RDWR|O_CREAT);
        if (fd < 0) {
            fprintf(stderr, "error %d\n", errno);
            exit(1);
        }
        posix_fadvise(fd, 0, 0, POSIX_FADV_RANDOM);
		assert(fd >= 0);
        numContentPages = 0;
        memset(header, 0, HEADER_SIZE);
    }
	// open existing file
    else { // at least header here
		fd = riot_open(pathname, O_RDWR);
        if (fd < 0) {
            fprintf(stderr, "error %d\n", errno);
            exit(1);
        }
        posix_fadvise(fd, 0, 0, POSIX_FADV_RANDOM);

		assert(fd >= 0);
		struct stat s;
		fstat(fd, &s);
        numContentPages = s.st_size/PAGE_SIZE - NUM_HEADER_PAGES;
        pread(fd, header, HEADER_SIZE, 0);
    }
}

// Remember to write the header back!
BitmapPagedFile::~BitmapPagedFile() {
    flush();
    fsync(fd);
    close(fd);
    freePageImage(header);
}

// only mark bit in header as allocated, defer writing until later
RC_t BitmapPagedFile::allocatePage(PID_t &pid) {
    // first check if next bit beyond currently allocated ones is available
    if(numContentPages < NUM_BITS_HEADER) { // still unused bits in header
        pid = numContentPages;
        allocate(pid);
        numContentPages++;
        return RC_OK;
    }

    // check for empty slot in allocated header space
	size_t numWords = NUM_BITS_HEADER / 32;
    for(size_t k = 0; k < numWords; ++k) {
		// check one word (4 bytes) at a time
        if(header[k] != ~(u32)0) {
            int bindex = findFirstZeroBit(header[k]);
			setBit(header[k], bindex);
            pid = k * 32 + bindex;
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
    if(pid >= NUM_BITS_HEADER) { // pid outside valid range
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

RC_t BitmapPagedFile::readPage(PageRec *rec) {
#ifdef PROFILING
    static timeval time1, time2;
    gettimeofday(&time1, NULL);
#endif
    // pid is not in usable region yet or is unallocated
    PID_t pid = rec->pid;
    if(pid >= numContentPages)
        return RC_OutOfRange;
    if(!isAllocated(pid)) 
        return RC_NotAllocated;
    ssize_t rd = pread(fd, rec->image, PAGE_SIZE, (NUM_HEADER_PAGES + pid)*PAGE_SIZE);
    if (rd != PAGE_SIZE)
        perror("BitmapPagedFile::readPage ");
#ifdef PROFILING
    gettimeofday(&time2, NULL);
    accessTime += time2.tv_sec - time1.tv_sec + (time2.tv_usec - time1.tv_usec)
        / 1000000.0 ;
    readCount++;
#endif
    return RC_OK;
}

// writes ph to file if pid has been allocated
RC_t BitmapPagedFile::writePage(PageRec *rec) {
#ifdef PROFILING
    static timeval time1, time2;
    gettimeofday(&time1, NULL);
#endif
    //PageRec *rec = (PageRec*) ph;
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
    ssize_t wc = pwrite(fd, rec->image, PAGE_SIZE, (NUM_HEADER_PAGES + pid)*PAGE_SIZE);
    if (wc != PAGE_SIZE)
        perror("BitmapPagedFile::writePage ");
#ifdef PROFILING
    gettimeofday(&time2, NULL);
    accessTime += time2.tv_sec - time1.tv_sec + (time2.tv_usec - time1.tv_usec)
        / 1000000.0 ;
    writeCount++;
#endif
    return RC_OK;
}

RC_t BitmapPagedFile::flush()
{
    // make sure number of pages in file is consistent with numContentPages
	struct stat s;
	fstat(fd, &s);
	size_t numTotalPages = NUM_HEADER_PAGES + numContentPages;
    if(numTotalPages > s.st_size/PAGE_SIZE) {
        int wc = pwrite(fd, header, PAGE_SIZE, (numTotalPages-1)*PAGE_SIZE); // write any data
        if (wc != PAGE_SIZE)
            perror("BitmapPagedFile::writePage ");
    }
    // write header
    int wc = pwrite(fd, header, HEADER_SIZE, 0);
    if (wc != HEADER_SIZE)
        perror("BitmapPagedFile::writePage ");
    return RC_OK;
}
