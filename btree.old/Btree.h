#ifndef BTREE_H
#define BTREE_H

#include "BtreeBlock.h"
#include "BtreeCursor.h"
#include "Splitter.h"
#include "../lower/PagedStorageContainer.h"
#include "../lower/BufferManager.h"

const int BtreeBufferSize = 100;

struct BtreeHeader {
    u32 endsBy;		/// all keys >=0 and < endsBy
	u32 nLeaves;	/// number of leaf nodes
	u32 depth;		/// depth of tree; root is at depth 0
	PID_t root;		/// PID of root
	PID_t firstLeaf;	/// PID of first leaf node
};

/*
 * A Btree is used to store (key,val) pairs in a one-dimensional array. The
 * keys starts from 0. It is stored in a paged storage container, e.g.,
 * BitmapPagedFile. The first (PID=0) page in the file contains the header
 * of the entire Btree, as defined in struct BtreeHeader.
 */
class Btree {
protected:
	// the underlying file storing the B-tree
	PagedStorageContainer *file;

	// buffer manager for this array; default LRU
	BufferManager<> *buffer;

    Splitter *internalSplitter;
    Splitter *leafSplitter;

private:
	// the header block(page) is kept in mem during lifetime of the tree
	// BtreeBlock *headerBlock;
    PageHandle headerPage;
    static const PID_t headerPID = 0;

    void split(BtreeCursor *cursor);

public:
	// points to the data in the header page
	BtreeHeader *header;

public:
	/**
	 * Creates a new Btree with the given file name and dimension
	 * size. If the file exists, it is overwritten. Given dim, all
	 * keys in the Btree should be >=0 and < endsby. The header page
	 * (0-th page in the file) is written immediately.
	 */
	Btree(const char *fileName, u32 endsBy, Splitter *leafSp, Splitter *intSp);

	/**
	 * Initializes a Btree from the given file. Metadata is read from the
	 * header page.
	 */
	Btree(const char *fileName, Splitter *leafSp, Splitter *intSp);

	/**
	 * Destroys the Btree object (also release header page).
	 */
	~Btree();

    int search(Key_t key, BtreeCursor *cursor);

	int put(Key_t &key, Datum_t &datum);
	int put(Key_t begin, Key_t end, Datum_t *data);
	int put(Key_t *keys, Datum_t *data, int length);

	int get(Key_t key, Datum_t *datum);
	int get(Key_t begin, Key_t end, Datum_t *data);
	int get(Key_t *keys, Datum_t *data, int length);

    void loadPage(PageHandle &ph);
    void releasePage(const PageHandle &ph);

    void setInternalSplitter(Splitter *sp) {internalSplitter = sp; }
    void setLeafSplitter(Splitter *sp) { leafSplitter = sp; }

    void print();
    u32  getNumLeaves() { return header->nLeaves; }
};

#endif
