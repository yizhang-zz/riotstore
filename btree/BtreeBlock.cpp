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

	// Initialize payload to NA for dense leaf block
	if (isLeaf && isDense) {
		int i = 0;
		Datum_t *p = (Datum_t*) (*ph.image + headerSize);
		for (; i < denseCap; i++) {
			p[i] = NA_DOUBLE;
		}
	}
}

/*
 * Searches the block for a particular key. If found, index of the entry is
 * written in idx and 0 is returned. Otherwise, idx is set to the index of
 * key if key is to be inserted and 1 is returned.
 */ 
int BtreeBlock::search(Key_t key, u16 *idx)
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
	} while (p <= q && pKeys[mid] != key);

	if (pKeys[mid] == key) {
		*idx = mid;
		return 0;
	}

	*idx = p;
	return 1;
}

/* 
 * Puts an entry into the block. The same key may already exist in the
 * block. 
 * 
 * Returns BT_OK if successfully inserted without overwriting or
 * overflowing, BT_OVERWRITE if a value exists and is overwritten,
 * BT_OVERFLOW if the insertion causes an overflow.
 */
int BtreeBlock::put(Key_t key, void *pDatum)
{
	Datum_t datum = *(Datum_t*)pDatum;

	// If dense leaf block:
	// no danger of overflow.
	if (isLeaf && isDense) {
		Datum_t *p = (Datum_t*) ((*ph.image)+headerSize);
		u16 offset = key - lower;
		if (ISNA(p[offset])) {
			p[offset] = datum;
			nEntries++;
			return BT_OK;
		}
		p[offset] = datum;
		return BT_OVERWRITE;
	}

	Key_t *pK = (Key_t*) ((*ph.image)+headerSize);
	Datum_t *pD = (Datum_t*) (*ph.image + PAGE_SIZE) - 1;
	PID_t *pP = (PID_t*) (*ph.image + PAGE_SIZE) - 1;

	u16 idx;
	int res = search(key, &idx);
	if (res == BT_FOUND) { // an overwrite
		if (isLeaf) { // sparse leaf
			memcpy(pD-idx, pDatum, sizeof(Datum_t));
		}
		else { // internal node
			memcpy(pP-idx, pDatum, sizeof(PID_t));
		}
		return BT_OVERWRITE;
	}
	else { // an insertion
		u16 cap = isLeaf ? sparseCap : internalCap ;
		if (nEntries >= cap) { // overflow
			OverflowEntry &e = overflowEntries[nEntries-cap];
			e.idx = idx;
			// sizeof(Datum_t) is safe to cover PID_t
			memcpy(e.data, pDatum, sizeof(Datum_t));
		}
		else {	// non-overflow
			// NOTE: unsigned type cannot be used for i because
			// nEntries can be 0 and assigning -1 to unsigned
			// causes error.
			int i;	

			for (i = nEntries-1; i >= idx; i--) {
				pK[i+1] = pK[i];
			}
			pK[idx] = key;
			if (isLeaf) {
				for (i=nEntries-1; i >= idx; i--)
					pD[-i-1] = pD[-i];
				memcpy(pD-idx, pDatum, sizeof(Datum_t));
			}
			else {
				for (i=nEntries-1; i >= idx; i--)
					pP[-i-1] = pP[-i];
				memcpy(pP-idx, pDatum, sizeof(PID_t));
			}
		}
		if (++nEntries > cap)
			return BT_OVERFLOW;
		else
			return BT_OK;
	}
}

/*
 * Gets an entry with the specified key. Returns BT_OK if found.
 */

int BtreeBlock::get(Key_t key, void *pRes)
{ 
	// For dense leaf block
	if (isLeaf && isDense) {
		Datum_t *p = (Datum_t*) ((*ph.image)+headerSize);
		u16 offset = key - lower;
		memcpy(pRes, p+offset, sizeof(Datum_t));
		return BT_OK;
	}

	Key_t *pK = (Key_t*) ((*ph.image)+headerSize);
	Datum_t *pD = (Datum_t*) (*ph.image + PAGE_SIZE) - 1;
	PID_t *pP = (PID_t*) (*ph.image + PAGE_SIZE) - 1;

	u16 idx;
	int res = search(key, &idx);
	if (res == BT_FOUND) { // an overwrite
		if (isLeaf) 
			memcpy(pRes, pD-idx, sizeof(Datum_t));
		else
			memcpy(pRes, pP-idx, sizeof(PID_t));
		return BT_OK;
	}
	else {
		if (isLeaf) {
			Datum_t val = BtreeBlock::defaultValue;
			memcpy(pRes, &val, sizeof(Datum_t));
			return BT_OK;
		}
		else
			return BT_NOT_FOUND;
	}
}
