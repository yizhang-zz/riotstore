#ifndef BITMAP_PAGED_FILE_H
#define BITMAP_PAGED_FILE_H

#include <stdio.h>
#include "../common/common.h"
#include "PagedStorageContainer.h"

//////////////////////////////////////////////////////////////////////
// A basic paged file implemented using a file header page containing
// a bitmap, followed by multiple content pages.  The header page
// stores a bitmap indicating which content pages are allocated and
// which are free.  The length of the bitmap is the number of content
// pages, which can be readily calculated by (size of the file / size
// of a page - 1).  The number of bits that can fit into the header
// page implicitly imposes an upper limit on the maximum number of
// content pages.

class BitmapPagedFile : public PagedStorageContainer {

private:

  FILE *file;
  uint32_t numContentPages;
  Byte_t header[PAGE_SIZE];
  //bitset<8*PAGE_SIZE> header;

public:

  BitmapPagedFile(FILE *file);

  // Remember to write the header back!
  virtual ~BitmapPagedFile();

  // Creates a BitmapPagedFile over a disk file of a given name.  If
  // the file doesn't exist yet, an empty paged file will be created
  // with the file header.
  static RC_t createPagedFile(const char *fileName, BitmapPagedFile *&pf);

  // The underlying file may expand if needed.
  virtual RC_t allocatePage(PID_t &pid);

  // The underlying file may expand if needed.
  virtual RC_t allocatePageWithPid(PID_t pid);

  virtual RC_t disposePage(PID_t pid);

  virtual RC_t readPage(PageHandle &ph);

  virtual RC_t writePage(const PageHandle &ph);

private:

  void setBit(int bytePos, int bitPos);

  void clearBit(int bytePos, int bitPos);

  int getBit(int bytePos, int bitPos);

};

inline void BitmapPagedFile::setBit(int bytePos, int bitPos) {
    header[bytePos] |= (1 << bitPos);
}

inline void BitmapPagedFile::clearBit(int bytePos, int bitPos) {
    header[bytePos] &= ~(1 << bitPos);
}

inline int BitmapPagedFile::getBit(int bytePos, int bitPos) {
    return (header[bytePos] & (1 << bitPos)) >> bitPos;
}
//////////////////////////////////////////////////////////////////////
// Another possible implementation of PagedStorageContainer would be
// IndexedPagedFile, where the header page stores, instead of a
// bitmap, a mapping from pid to the actual page sequence number in
// the content pages.  This mapping allows page with a large pid to be
// allocated without allocating and writing pages with smaller pids.
// This feature would come in handy for DirectlyMappedArray when
// insertions into a large array come in random order, because an
// insertion near the end of the array won't cause the entire array to
// be materialized.
//
// On the other hand, it seems that some file systems do support
// seeking beyond the end of a file and writing some data there,
// without writing or allocating the pages in between (even though
// C++'s I/O library doesn't impose such a feature as a standard).
// Therefore, we will simply implement DirectlyMappedArray on top of a
// BitmapPagedFile.

#endif
