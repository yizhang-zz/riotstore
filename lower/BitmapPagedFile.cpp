#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "BitmapPagedFile.h"
using namespace std;

/* 
   bit ordering in header: 7,6,5,4,3,2,1,0|7,6,5,4,3,2,1,0|7,6....
   a bit expressed as (k, n)tuple where the bit is contained within the kth
   byte in header and is the nth most significant bit (0 being least
   significant) of that byte.
   define pid = 8*k + n
   when a new page is needed, the smallest available pid is assigned.
   numContentPages <= 8*PAGE_SIZE --> maximum number of pages in file container
   maximum allocated pid < numContentPages
   pid to bitmask mapping:
   7,6,5,4,3,2,1,0|15,14,13,12,11,10,9,8|23,22,...
   header is allocated 1 byte at a time. (when first 8 bits are used and need to
   allocate another, next 8 bits are cleared).
 */

BitmapPagedFile::BitmapPagedFile(FILE *f) {
    file = f;
    fseek(file, 0, SEEK_END);
    if(ftell(file) == 0) { // new file
        numContentPages = 0;
        memset(header, 0, PAGE_SIZE);
        fwrite(header, PAGE_SIZE, 1, file);
        fflush(file);
    }
    else { // at least header here
        numContentPages = ftell(file)/PAGE_SIZE - 1;
        rewind(file);
        fread(header, PAGE_SIZE, 1, file);
    }
}

// Remember to write the header back!
BitmapPagedFile::~BitmapPagedFile() {
    // make sure number of pages in file is consistent with numContentPages
    fflush(file);
    fseek(file, 0, SEEK_END);
    Byte_t *filler = header; // use header as filler page
    int pagesToAdd = 1 + numContentPages - ftell(file)/PAGE_SIZE;
    for(int k = 0; k < pagesToAdd; k++) {
        fwrite(filler, PAGE_SIZE, 1, file);
    }
    rewind(file);
    fwrite(header, PAGE_SIZE, 1, file);
    fclose(file);
}

/* Creates a BitmapPagedFile over a disk file of a given name.  If 
   the file doesn't exist yet, an empty paged file will be created
   with the file header.
 */
RC_t BitmapPagedFile::createPagedFile(const char *fileName, BitmapPagedFile
        *&pf) {
    FILE *file = fopen(fileName, "rb+");
    if(file == NULL) { // unable to find existing file
        file = fopen(fileName, "wb+");    // create new file for read/write
    }
    pf = new BitmapPagedFile(file);
    return RC_SUCCESS;
}

/* only mark bit in header as allocated, defer writing until later */
RC_t BitmapPagedFile::allocatePage(PID_t &pid) {
    // first check for empty slot in allocated header space
    for(int k = 0; k < numContentPages; k++) {
        if(k%8 == 0 && (header[k/8] & 255) == 255) { // check 1 byte at a time
            k += 7;  /* advance 1 byte (k will be incremented at end of pass to
                        account for all 8 bits */
            continue;
        }
        if(!isAllocated(k)) { // found empty slot
            pid = k;
            allocate(pid);
            return RC_SUCCESS;
        }
    }
    /*
       if(numContentPages%8 != 0) { // still unused bits in last used byte
       pid = numContentPages;
       numContentPages++;
       allocate(pid);
       return RC_SUCCESS;
       }
     */
    if(numContentPages < 8*PAGE_SIZE) { // still unused bytes in header
        //header[numberContentPages/8] = 0;
        pid = numContentPages;
        numContentPages++;
        allocate(pid);
        return RC_SUCCESS;
    }

    return RC_FAILURE; // header filled 
}

/* tries to allocate new page with input pid. if pid is available, bitmask flag
 * is made. otherwise returns failure. The page is not only allocated in memory
 * and not written until later call
 */
RC_t BitmapPagedFile::allocatePageWithPID(PID_t pid) {
    if(pid >= 8*PAGE_SIZE) { // pid outside valid range
        return RC_FAILURE;
    }

    // if pid is within range of current allocated pages
    if(pid < numContentPages) {
        if(isAllocated(pid)) { // pid already allocated
            return RC_FAILURE;
        }
        allocate(pid);
        return RC_SUCCESS;
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
    return RC_SUCCESS;
}

RC_t BitmapPagedFile::disposePage(PID_t pid) {
    if(pid >= numContentPages) {
        return RC_FAILURE; /* pid not within allocated pid range, nothing is done */
    }

    if(isAllocated(pid)) {
        deallocate(pid); /* deallocates mapped bit in header, defers actual
                            physical disposal of data until later */
        return RC_SUCCESS;
    }
    return RC_FAILURE;
}

RC_t BitmapPagedFile::readPage(PageHandle &ph) {
    // pid is not in usable region yet or is unallocated
    if(ph.pid >= numContentPages || !isAllocated(ph.pid)) { 
        return RC_FAILURE; 
    }

    fseek(file, (1 + ph.pid)*PAGE_SIZE, SEEK_SET); // +1 for header page
    fread(ph.image, PAGE_SIZE, 1, file);
    return RC_SUCCESS;
}

// writes ph to file if pid has been allocated
RC_t BitmapPagedFile::writePage(const PageHandle &ph) {
    // pid is not in usable region yet or is unallocated
    if(ph.pid >= numContentPages || !isAllocated(ph.pid)) { 
        return RC_FAILURE;
    }

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
    fseek(file, (1 + ph.pid)*PAGE_SIZE, SEEK_SET); // +1 for header page
    fwrite(ph.image, PAGE_SIZE, 1, file);
    return RC_SUCCESS;
}
