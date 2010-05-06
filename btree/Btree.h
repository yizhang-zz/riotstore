#ifndef BTREE_H
#define BTREE_H

#include "../lower/LinearStorage.h"
#include "BtreeBlock.h"
#include "BtreeCursor.h"
#include "Splitter.h"
#include "BtreePagePacker.h"

namespace Btree
{

#ifdef USE_BATCH_BUFFER
  class BatchBuffer;
  class BtreeStat;
#endif

struct BTreeHeader {
    u32 endsBy;		/// all keys >=0 and < endsBy
	u32 nLeaves;	/// number of leaf nodes
	u32 depth;		/// depth of tree; root is at depth 0
	PID_t root;		/// PID of root
	PID_t firstLeaf;	/// PID of first leaf node
};

/*
 * A BTree is used to store (key,val) pairs in a one-dimensional array. The
 * keys starts from 0. It is stored in a paged storage container, e.g.,
 * BitmapPagedFile. The first (PID=0) page in the file contains the header
 * of the entire BTree, as defined in struct BTreeHeader.
 */
class BTree : public LinearStorage {
public:
    static int BufferSize;
    static int getBufferSize();

protected:
    BtreePagePacker *packer;
    Splitter *internalSplitter;
    Splitter *leafSplitter;

private:
	// the header block(page) is kept in mem during lifetime of the tree
	// BTreeBlock *headerBlock;
    PageHandle headerPage;
    static const PID_t headerPID = 0;

    void split(Cursor *cursor);

public:
	// points to the data in the header page
	BTreeHeader *header;
#ifdef USE_BATCH_BUFFER
  BatchBuffer *batbuf;
  BtreeStat *stat;
#endif

public:
	/**
	 * Creates a new BTree with the given file name and dimension
	 * size. If the file exists, it is overwritten. Given dim, all
	 * keys in the BTree should be >=0 and < endsby. The header page
	 * (0-th page in the file) is written immediately.
	 */
	BTree(const char *fileName, u32 endsBy, Splitter *leafSp, Splitter *intSp);

	/**
	 * Initializes a BTree from the given file. Metadata is read from the
	 * header page.
	 */
	BTree(const char *fileName, Splitter *leafSp, Splitter *intSp);

	/**
	 * Destroys the BTree object (also release header page).
	 */
	~BTree();

    int search(Key_t key, Cursor *cursor);

	int put(const Key_t &key, const Datum_t &datum);
	//int put(Key_t begin, Key_t end, Datum_t *data);
	//int put(Key_t *keys, Datum_t *data, int length);

	int get(const Key_t &key, Datum_t &datum);
	//int get(Key_t begin, Key_t end, Datum_t *data);
	//int get(Key_t *keys, Datum_t *data, int length);

    ArrayInternalIterator *createIterator(IteratorType t, Key_t &beginsAt, Key_t &endsBy);

    void setInternalSplitter(Splitter *sp) {internalSplitter = sp; }
    void setLeafSplitter(Splitter *sp) { leafSplitter = sp; }

    void print();
    u32  getNumLeaves() { return header->nLeaves; }

    void allocatePage(PID_t &pid, PageHandle &ph) {
        buffer->allocatePage(ph);
        pid = buffer->getPID(ph);
    }
    PageHandle loadPage(PID_t pid) {
        PageHandle ph;
        buffer->readPage(pid, ph);
        return ph;
    }
    void releasePage(PageHandle ph) {
        buffer->unpinPage(ph);
    }
    void *getPagePacked(PageHandle ph) {
        return buffer->getPageImage(ph);
    }
    void *getPageUnpacked(PageHandle ph) {
        return buffer->getUnpackedPageImage(ph);
    }

};
}
#endif
