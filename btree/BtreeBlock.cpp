#include "../common/common.h"
#include "BtreeBlock.h"

/* 
 * Initialize static members.
 */
/*
u16 BtreeBlock::denseCap = (PAGE_SIZE -
		BtreeBlock::headerSize)/(sizeof(Datum_t));
u16 BtreeBlock::sparseCap = (PAGE_SIZE -
		BtreeBlock::headerSize)/(sizeof(Key_t)+sizeof(Datum_t));
u16 BtreeBlock::internalCap = (PAGE_SIZE -
		BtreeBlock::headerSize)/(sizeof(Key_t)+sizeof(PID_t));
*/


/*
 * Initializes the block by reading from a page image. The key range is
 * provided since the caller, with the knowledge of the overall tree, should
 * know the exact key range that this block is responsible for.
 */
BtreeBlock::BtreeBlock(PageHandle *pPh, Key_t beginsAt, Key_t endsBy,
                       bool create)
{
	ph = *pPh;		// make a physical copy
	lower = beginsAt;
	upper = endsBy;

	u8 *pData = *ph.image;
    nEntries = (u16*) (pData+1);
    
    if (create) {
        *nEntries = 0;
    }
}

BtreeBlock *BtreeBlock::load(PageHandle *pPh, Key_t beginsAt, Key_t endsBy)
{
    BtreeBlock *block;
    u8 flags = (*pPh->image)[0];
    if (flags & 1) {		// leaf
        if (flags & 2) {	// dense
            block = new BtreeDLeafBlock(pPh, beginsAt, false);
        } else {
            block = new BtreeSLeafBlock(pPh, beginsAt, endsBy, false);
        }
    } else {
        block = new BtreeIntBlock(pPh, beginsAt, endsBy, false);
    }
    return block;
}

BtreeIntBlock::BtreeIntBlock(PageHandle *pPh, Key_t beginsAt, Key_t endsBy,
                             bool create)
    : BtreeSparseBlock(pPh, beginsAt, endsBy, create)
{
    u8 *pData = *ph.image;
    rightChild = (PID_t*) (pData+3);

    if (create) {
        u8 *flag = *ph.image;
        *flag = 0;
    }
}

BtreeDLeafBlock::BtreeDLeafBlock(PageHandle *pPh, Key_t beginsAt,
                                 bool create)
    : BtreeBlock(pPh, beginsAt, beginsAt+capacity, create)
{
    u8 *pData = *ph.image;
    nextLeaf = (PID_t*) (pData+3);

    if (create) {
        u8 *flag = *ph.image;
        *flag = 3;
        
        // Init payload to defaultValue
        Datum_t *p = (Datum_t*) (*ph.image + headerSize);
        for(int i=0; i<capacity; i++) {
            p[i] = defaultValue;
        }
    }
}

BtreeSLeafBlock::BtreeSLeafBlock(PageHandle *pPh, Key_t beginsAt, Key_t endsBy,
                                 bool create)
    : BtreeSparseBlock(pPh, beginsAt, endsBy, create)
{
    u8 *pData = *ph.image;
    nextLeaf = (PID_t*) (pData+3);

    if (create) {
        u8 *flag = *ph.image;
        *flag = 1;
    }
}


/*
 * Updates the block header in the page image.
 */

/*
void BtreeBlock::syncHeader()
{
	u8 *pData = *ph.image;
	*pData |= isLeaf;
	*pData |= isDense;

	*((u16*)(pData + 1)) = nEntries;

	if (isLeaf) {
		*((PID_t*)(pData+3)) = ptr.nextLeaf;
	}
	else {
		*((PID_t*)(pData+3)) = ptr.rightChild;
	}
}
*/

int BtreeDLeafBlock::search(Key_t key, u16 *idx)
{
    assert(key>=lower && key<upper);
    *idx = key - lower;
    return BT_OK;
}

/*
 * Searches the block for a particular key. If found, index of the entry is
 * written in idx and 0 is returned. Otherwise, idx is set to the index of
 * key if key is to be inserted and 1 is returned.
 */ 
int BtreeSparseBlock::search(Key_t key, u16 *index)
{
	assert(key>=lower && key<upper);
	if (*nEntries == 0) {
		*index = 0;
		return BT_NOT_FOUND;
	}

	// binary search on the keys
	int p = 0;
	int q = *nEntries - 1;
	int mid;
	Key_t *pKeys = (Key_t*) (*ph.image+headerSize);

	do {
		mid = (p+q)/2;
		if (pKeys[mid] > key) q = mid-1;
		else  p = mid+1;
	} while (p <= q && pKeys[mid] != key);

	if (pKeys[mid] == key) {
		*index = mid;
		return BT_OK;
	}

	*index = p;
	return BT_NOT_FOUND;
}

/* 
 * Puts an entry into the block. The same key may already exist in the
 * block.
 *
 * If the data to be inserted is defaultValue for leaf blocks, it is
 * equivalent to a delete. BT_OK is returned. 
 * 
 * Returns BT_OK if successfully inserted without overwriting or
 * overflowing, BT_OVERWRITE if a value exists and is overwritten,
 * BT_OVERFLOW if the insertion causes an overflow.
 */
int BtreeDLeafBlock::put(Key_t key, void *pDatum)
{
	Datum_t datum = *(Datum_t*)pDatum;
	if (IS_DEFAULT(datum)) {
		del(key);
		return BT_OK;
	}
    
    Datum_t *p = (Datum_t*) ((*ph.image)+headerSize);
    u16 offset = key - lower;
    if (IS_DEFAULT(p[offset])) {
        p[offset] = datum;
        ++(*nEntries);
        return BT_OK;
    }
    p[offset] = datum;
    return BT_OVERWRITE;
}

int BtreeSparseBlock::put(Key_t key, void *pDatum)
{
	Key_t *pK = (Key_t*) ((*ph.image)+headerSize);
	u8 *p =  *ph.image + PAGE_SIZE - getDatumSize();

	u16 idx;
	int res = search(key, &idx);
    size_t size = getDatumSize();
    u16 &n = *nEntries;
	if (res == BT_OK) { // an overwrite
        memcpy(p-idx*size, pDatum, size);
		return BT_OVERWRITE;
	}
	else { // an insertion
		if (n >= getCapacity()) { // overflow
			OverflowEntry &e = overflowEntries[n-getCapacity()];
			e.idx = idx;
			// sizeof(Datum_t) is safe to cover PID_t
			memcpy(e.data, pDatum, size);
		}
		else {	// non-overflow
			// NOTE: unsigned type cannot be used for i because
			// nEntries can be 0 and assigning -1 to unsigned
			// causes error.
			int i;	

			for (i = n-1; i >= idx; i--) {
				pK[i+1] = pK[i];
			}
			pK[idx] = key;
            for (i = n-1; i >= idx; i--)
				memcpy(p-(i+1)*size, p-i*size, size);
                //pD[-i-1] = pD[-i];
            memcpy(p-idx*size, pDatum, size);
		}
		if (++n > getCapacity())
			return BT_OVERFLOW;
		else
			return BT_OK;
	}
}

/*
 * Gets an entry with the specified key. Returns BT_OK if found,
 * BT_NOT_FOUND otherwise. For dense leaf blocks, BT_OK is always returned.
 * For sparse leaf blocks, BT_OK is returned even if the key does not exist,
 * because we know it's a default value and is not stored explicitly. For an
 * internal node, BT_NOT_FOUND is returned if the key cannot be found.
 */

int BtreeDLeafBlock::get(Key_t key, void *pRes)
{ 
    Datum_t *p = (Datum_t*) ((*ph.image)+headerSize);
    u16 offset = key - lower;
    memcpy(pRes, p+offset, sizeof(Datum_t));
    return BT_OK;
}

int BtreeSparseBlock::get(Key_t key, void *pRes)
{
	Key_t *pK = (Key_t*) ((*ph.image)+headerSize);
    // pointing to the begining of the last datum/pid
    u8 *p = (*ph.image+PAGE_SIZE) - getDatumSize();
	//Datum_t *pD = (Datum_t*) (*ph.image + PAGE_SIZE) - 1;
	//PID_t *pP = (PID_t*) (*ph.image + PAGE_SIZE) - 1;

	u16 idx;
	int res = search(key, &idx);
	if (res == BT_OK) {
        memcpy(pRes, p-idx*getDatumSize(), getDatumSize());
		return BT_OK;
	}
	else {
		// if (isLeaf) {
		// 	Datum_t val = BtreeBlock::defaultValue;
		// 	memcpy(pRes, &val, sizeof(Datum_t));
		// 	return BT_OK;
		// }
		// else
        return BT_NOT_FOUND;
	}
}

/*
 * Deletes an entry with the specified key. If such a key is not stored,
 * just return BT_OK for leaf blocks, or BT_NOT_FOUND for internal blocks. 
 */

int BtreeDLeafBlock::del(Key_t key)
{
    Datum_t *p = (Datum_t*) (*ph.image+headerSize);
    u16 offset = key - lower;
    if (!IS_DEFAULT(p[offset])) {
        p[offset] = defaultValue;
        --(*nEntries);
    }
    return BT_OK;
}

int BtreeSparseBlock::del(Key_t key)
{
	Key_t *pK = (Key_t*) ((*ph.image)+headerSize);
    u8 *pD = (*ph.image + PAGE_SIZE) - getDatumSize();
    u16 &n = *nEntries;
    int unitSize = getDatumSize();
    
	//Datum_t *pD = (Datum_t*) (*ph.image + PAGE_SIZE) - 1;
	//PID_t *pP = (PID_t*) (*ph.image + PAGE_SIZE) - 1;

    /* Shift all entries with keys in (key, upper) to the left. A more
     * efficient implementation is to allow "holes", but that
     * complicates searching and insertion.
     */
    u16 idx;
    if (search(key, &idx) == BT_OK) {
        int i;
        for (i = idx; i < n-1; i++)
            pK[i] = pK[i+1];
        u8 *end = pD - idx * unitSize;
        u8 *start = pD - (n-1) * unitSize;
        while (start != end)
            *(start--) = *(start-unitSize);
        --n;
        return BT_OK;
    }
    return BT_NOT_FOUND;

	/*
	 * Internal blocks: if key has index idx, then both key and the idx-th
	 * child pointer are removed. Remember that keys[idx-1] <= p[idx] <
	 * keys[idx].
	 */
    /*
      u16 idx;
      if (search(key, &idx) == BT_OK) {
      int i;
      for (i = idx; i < nEntries-1; i++)
      pK[i] = pK[i+1];
      for (i = idx; i < nEntries-1; i++)
      pP[-i] = pP[-i-1];
      nEntries--;
      return BT_OK;
      }
      else
      return BT_NOT_FOUND;
    */
}
