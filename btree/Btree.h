#ifndef BTREE_H
#define BTREE_H

#include "BtreeBlock.h"
#include "../lower/BitmapPagedFile.h"
#include "../lower/BufferManager.h"

struct BtreeHeader {
	u32 dimension;	// all keys >=0 and < dimension
	u32 nLeaves;	// number of leaf nodes
	u32 depth;		// root is at depth 0
	PID_t root;
	PID_t firstLeaf;
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
	BitmapPagedFile *file;

	// buffer manager for this array; default LRU
	BufferManager<> *buffer;

private:
	// the header block(page) is kept in mem during lifetime of the tree
	BtreeBlock *headerBlock;

public:
	// points to the data in the header page
	BtreeHeader *header;

public:
	/*
	 * Creates a new Btree with the given file name and dimension size. If
	 * the file exists, it is overwritten. Given dim, all keys in the Btree
	 * should be >=0 and < dim. The header page (0-th page in the file) is
	 * written immediately.
	 */
	Btree(const char *fileName, u32 dim);

	/*
	 * Initializes a Btree from the given file. Metadata is read from the
	 * header page.
	 */
	Btree(const char *fileName);

	/*
	 * Destroys the Btree object (also release header page).
	 */
	~Btree();

	int put(Key_t key, Datum_t *datum);
	int put(Key_t begin, Key_t end, Datum_t *data);
	int put(Key_t *keys, Datum_t *data, int length);

	int get(Key_t key, Datum_t *datum);
	int get(Key_t begin, Key_t end, Datum_t *data);
	int get(Key_t *keys, Datum_t *data, int length);
};

#endif
