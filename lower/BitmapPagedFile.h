#ifndef BITMAP_PAGED_FILE_H
#define BITMAP_PAGED_FILE_H

#include "../common/common.h"
#include "PagedStorageContainer.h"
#include "PageRec.h"

//////////////////////////////////////////////////////////////////////
// A basic paged file implemented using a file header page containing
// a bitmap, followed by multiple content pages.  The header page
// stores a bitmap indicating which content pages are allocated and
// which are free.  The length of the bitmap is the number of content
// pages, which can be readily calculated by (size of the file / size
// of a page - 1).  The number of bits that can fit into the header
// page implicitly imposes an upper limit on the maximum number of
// content pages.

// Assuming a pagesize of 4KB, a single header page can support a
// maximum of 4K*8*4KB=128MB data. This seems too small for practical use
class BitmapPagedFile : public PagedStorageContainer {
public:
	const static size_t NUM_HEADER_PAGES;
	const static size_t NUM_BITS_PER_PAGE;
	const static size_t HEADER_SIZE;
	const static size_t NUM_BITS_HEADER;

  BitmapPagedFile(const char *pathname, int flag);

  // Remember to write the header back!
  virtual ~BitmapPagedFile();

  // Creates a BitmapPagedFile over a disk file of a given name.  If
  // the file doesn't exist yet, an empty paged file will be created
  // with the file header.
  // static RC_t create(const char *fileName, BitmapPagedFile *&pf);

  // The underlying file may expand if needed.
  virtual RC_t allocatePage(PID_t &pid);

  // The underlying file may expand if needed.
  virtual RC_t allocatePageWithPID(PID_t pid);

  virtual RC_t disposePage(PID_t pid);

  virtual RC_t readPage(PageRec *);

  virtual RC_t writePage(PageRec *);

  virtual RC_t flush();
private:
  u32 *header;
  int fd;	// file descriptor
  u32 numContentPages;

	// sets bit in header that maps to pid
	void setBit(u32 &word, int pos)
	{
		word |= (1 << pos);
	}

	// clears bit in header that maps to pid
	void unsetBit(u32 &word, int pos)
	{
		word &= ~(1 << pos);
	}

	void allocate(PID_t pid)
	{
		setBit(header[pid>>5], pid&31);
	}

	void deallocate(PID_t pid)
	{
		unsetBit(header[pid>>5], pid&31);
	}
	
	// returns value of bit in header that maps to pid
	bool isAllocated(PID_t pid)
	{
		if(pid < NUM_BITS_HEADER) {
			return (header[pid>>5] & (1 << (pid&31)));
		}
		return false;
	}
};

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
