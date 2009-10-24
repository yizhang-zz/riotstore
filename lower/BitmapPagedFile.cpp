
#include <stdio.h>
#include <stdlib.h>
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

BitmapPagedFile::BitmapPagedFile(FILE *file) {
   this.file = file;
   header = (Byte_t*) malloc(sizeof(PAGE_SIZE));
   fseek(file, 0, SEEK_END);
   if((numContentPages = ftell(file)/PAGE_SIZE - 1) >= 0) { // header exists
      rewind(file);
      fread(header, PAGE_SIZE, 1, file);
   }
   else { // new file with nothing allocated: 0 content pages, header
         // unallocated
      numContentPages = 0;
   }
}

// Remember to write the header back!
BitmapPagedFile::~BitmapPagedFile() {
   fflush(file);
   fwrite(header, sizeof(Byte_t), PAGE_SIZE/sizeof(Byte_t), file);
   fclose(file);
   free(header);
}

// Creates a BitmapPagedFile over a disk file of a given name.  If
// the file doesn't exist yet, an empty paged file will be created
// with the file header.
RC_t BitmapPagedFile::createPagedFile(const char *fileName, BitmapPagedFile
      *&pf) {
   if((file = fopen(fileName, "r+")) == 0) { // check if file does not exist
      file = fopen(fileName, "w+");    // create new file for read/write
   }
   pf = new BitmapPagedFile(file);
   return SUCCESS;
}

/* only mark bit in header as allocated, defer writing until later */
RC_t BitmapPagedFile::allocatePage(PID_t &pid) {
   // first check for empty slot in allocated header space
   for(int k = 0; k < numContentPages; k++) {
      if((header[k/8] & 255) == 255) { // check 1 byte at a time
         k += 7;  // advance 1 byte (k will be incremented at end of pass to
                  // account for all 8 bits
         continue;
      }
      if(getBit(k/8, k%8) == 0) { // found empty slot
         pid = k;
         setBit(pid/8, pid%8);
         return SUCCESS;
      }
   }

   if(numContentPages%8 != 0) { // still unused bits in last used byte
      pid = numContentPages;
      numContentPages++;
      setBit(pid/8, pid%8);
      return SUCCESS;
   }

   if(numContentPages < 8*PAGE_SIZE) { // still unused bytes in header
      header[numberContentPages/8] = 0;
      pid = numContentPages;
      numContentPages++;
      setBit(pid/8, pid%8);
      return SUCCESS;
   }

   return FAILURE; // header filled 
}

/* tries to allocate new page with input pid. if pid is available, bitmask flag
 * is made. otherwise returns failure. The page is not only allocated in memory
 * and not written until later call
 */
RC_t BitmapPagedFile::allocatePageWithPid(PID_t pid) {
   if(pid < numContentPages) {
      if(getBit(pid/8, pid%8) == 0) { // pid not used
         setBit(bytePos, bitPos);
         return SUCCESS;
      }
      return FAILURE; // pid already in use
   }

   if(pid < 8*PAGE_SIZE) { // pid within maximum bound
      int bytes = (pid - numContentPages)/8;
      for(; numContentPages < pid; numContentPages++) { // check this
         if(numContentPages%8 == 0) {
            header[numContentPages/8] = 0;
         }
      }
      setBit(pid/8, pid%8);
      return SUCCESS;
   }
   return FAILURE; // pid already allocated
}

RC_t BitmapPagedFile::disposePage(PID_t pid) {
   if(pid < numContentPages) {
      clearBit(pid/8, pid%8);
      return SUCCESS;
   }
   return FAILURE; // pid unallocated
}

RC_t BitmapPagedFile::readPage(PageHandle &ph) {
   if(getBit((ph.pid)/8, (ph.pid)%8) == 1) { // check if page is allocated
      fseek(file, (ph.pid+1)*PAGE_SIZE, SEEK_SET);
      fread(ph.image, PAGE_SIZE, 1, file);
      return SUCCESS;
   }
   return FAILURE;
}

RC_t BitmapPagedFile::writePage(const PageHandle &ph) {
   if(getBit((ph.pid)/8, (ph.pid)%8) == 1) { // check if page is allocated
      fseek(file, (ph.pid+1)*PAGE_SIZE, SEEK_SET);
      fwrite(ph.image, PAGE_SIZE, 1, file);
      fflush(file);
      return SUCCESS;
   }
   return FAILURE;
}

