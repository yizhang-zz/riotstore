#include "../common/common.h"
#include "BtreeBlock.h"

/* 
 * Initialize static members.
 */
u16 BtreeBlock::denseCap = (PAGE_SIZE -
		BtreeBlock::headerSize)/(sizeof(Datum_t));
u16 BtreeBlock::sparseCap = (PAGE_SIZE -
		BtreeBlock::headerSize)/(sizeof(Key_t)+sizeof(Datum_t));
u16 BtreeBlock::internalCap = (PAGE_SIZE -
		BtreeBlock::headerSize)/(sizeof(Key_t)+sizeof(PID_t));

/*
 * Initializes the block by reading from a page image. The key range is
 * provided since the caller, with the knowledge of the overall tree, should
 * know the exact key range that this block is responsible for.
 */
BtreeBlock::BtreeBlock(PageHandle *pPh, Key_t beginsAt, Key_t endsBy)
{
	ph = *pPh;		// make a physical copy
	lower = beginsAt;
	upper = endsBy;

	u8 *pData = *ph.image;
	u8 flags = pData[0];
	isLeaf = flags & 0x1;
	isDense = flags & 0x10;
	nEntries = *((u16*)(pData + 1));
	if (isLeaf) {
		nextLeaf = (PID_t*)(pData+3);
	}
	else {
		rightChild = (PID_t*)(pData+3);
	}
}

/* 
 * Initializes the block as new.
 */
BtreeBlock::BtreeBlock(PageHandle *pPh, Key_t beginsAt, Key_t endsBy, bool
		leaf, bool dense, bool root)
{
	ph = *pPh;
	lower = beginsAt;
	upper = endsBy;
	isLeaf = leaf;
	isDense = dense;
	nEntries = 0;

	// write flags
	u8 *pData = *ph.image;
	(*pData) |= leaf;
	(*pData) |= ((u8) dense) << 1;

	// write nEntries
	pData++;
	u16 *x = (u16*) pData;
	*x = nEntries;		// initializes to 0
}

/*
 * Searches the block for a particular key. If found, index of the entry is
 * written in idx and 0 is returned. Otherwise, idx is set to the index of
 * key if key is to be inserted and 1 is returned.
 */ 
int search(Key_t key, int *idx)
{
	assert(key>=lower && key<upper);

	if (isDense) {
		*idx = key - lower;
		return 0;
	}

	if (nEntries == 0) {
		*idx = 0;
		return 1;
	}

	// binary search on the keys
	int p = 0;
	int q = nEntries - 1;
	int mid;
	u8 *pData = *ph.image;
	Key_t *pKeys = (Key_t*) (pData+headerSize);

	do {
		mid = (p+q)/2;
		if (pKeys[mid] > key) q = mid-1;
		else  p = mid+1;
	} while (p <= q && pkeys[mid] != key);

	if (pKeys[mid] == key) {
		*idx = mid;
		return 0;
	}

	*idx = p;
	return 1;
}
