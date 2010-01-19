#include "BtreeSparseBlock.h"
#include "BtreeSLeafBlock.h"
#include "BtreeIntBlock.h"

/*
 * Searches the block for a particular key. If found, index of the entry is
 * written in idx and 0 is returned. Otherwise, idx is set to the index of
 * key if key is to be inserted and 1 is returned.
 */ 
int BtreeSparseBlock::search(Key_t key, int &index)
{
	assert(key>=lower && key<upper);
	if (*nEntries == 0) {
		index = 0;
		return BT_NOT_FOUND;
	}

	// binary search on the keys
	int p = 0;
	int q = *nEntries - 1;
	int mid;
	Key_t *pKeys = (Key_t*) payload;

	do {
		mid = (p+q)/2;
		if (pKeys[mid] > key) q = mid-1;
		else  p = mid+1;
	} while (p <= q && pKeys[mid] != key);

	if (pKeys[mid] == key) {
		index = mid;
		return BT_OK;
	}

	index = p;
	return BT_NOT_FOUND;
}

int BtreeSparseBlock::put(Key_t key, const void *pDatum)
{
	Key_t *pK = (Key_t*) payload;
	u8 *p =  *ph.image + PAGE_SIZE - getDatumSize();
    size_t size = getDatumSize();

	int index;
    Entry e;
    e.key = key;
    memcpy(&e.value, pDatum, size);
	int res = search(key, index);
    return put(index, e);

    // TODO: the following is obsolete
    int idx;
    res = search(key, idx);
    u16 &n = *nEntries;
	if (res == BT_OK) {
        memcpy(p-idx*size, pDatum, size);
		return BT_OVERWRITE;
	}
	else { // an insertion
		if (n >= getCapacity()) { // overflow
			OverflowEntry &e = overflowEntries[n-getCapacity()];
			e.index = idx;
            e.entry.key = key;
			memcpy(&e.entry.value, pDatum, size);
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

int BtreeSparseBlock::get(Key_t key, void *pRes)
{
    Key_t *pK = (Key_t*) payload;
    // pointing to the begining of the last datum/pid
    u8 *p = (*ph.image+PAGE_SIZE) - getDatumSize();
    //Datum_t *pD = (Datum_t*) (*ph.image + PAGE_SIZE) - 1;
    //PID_t *pP = (PID_t*) (*ph.image + PAGE_SIZE) - 1;

    int idx;
    int res = search(key, idx);
    if (res == BT_OK) {
        memcpy(pRes, p-idx*getDatumSize(), getDatumSize());
        return BT_OK;
    }
    else {
        return BT_NOT_FOUND;
    }
}

int BtreeSparseBlock::del(int index)
{
	Key_t *pK = (Key_t*) payload;
    u8 *pD = (*ph.image + PAGE_SIZE) - getDatumSize();
    int unitSize = getDatumSize();
    
    /* Shift all entries with keys in (key, upper) to the left. A more
     * efficient implementation is to allow "holes", but that
     * complicates searching and insertion.
     */
    assert(index >=0 && index < *nEntries);
    int i;
    for (i = index; i < *nEntries-1; i++)
        pK[i] = pK[i+1];
    u8 *end = pD - index * unitSize;
    u8 *start = pD - (*nEntries-1) * unitSize;
    while (start != end)
        *(start--) = *(start-unitSize);
    *nEntries -= 1;
    return BT_OK;
}

Key_t BtreeSparseBlock::getKey(int index)
{
    assert(index >= 0);
    if (index >= *nEntries) {
        return  upper;
    }
    
    Key_t *pK = (Key_t*) payload;
    Key_t key;
    // check overflow
    if (*nEntries > getCapacity()) {
        OverflowEntry &e = overflowEntries[0];
        if (index == e.index) {
            key = e.entry.key;
        }
        else if (index < e.index) {
            key = pK[index];
        }
        else {
            key = pK[index-1];
        }
    }
    else {
        key = pK[index];
    }
    return key;
}

Value BtreeSparseBlock::getValue(int index)
{
    assert(index >= 0 && index < *nEntries);
    Value val;
    u8* p = *ph.image + PAGE_SIZE - getDatumSize();
    size_t size = getDatumSize();
    // check overflow
    if (*nEntries > getCapacity()) {
        OverflowEntry &e = overflowEntries[0];
        if (index == e.index)
            memcpy(&val, &e.entry.value, size);
        else if (index < e.index)
            memcpy(&val, p-index*size, size);
        else
            memcpy(&val, p-(index-1)*size, size);
    }
    else
        memcpy(&val, p-index*size, size);
    return val;
}

int BtreeSparseBlock::get(int index, Entry &entry)
{
    assert(index >= 0 && index < *nEntries);
    Key_t *pK = (Key_t*) payload;
    u8* pD = *ph.image + PAGE_SIZE - getDatumSize();
    size_t size = getDatumSize();
    // check overflow
    if (*nEntries > getCapacity()) {
        OverflowEntry &e = overflowEntries[0];
        if (index == e.index)
            entry = e.entry;
        else if (index < e.index) {
            entry.key = pK[index];
            memcpy(&entry.value, pD-index*size, size);
        }
        else {
            entry.key = pK[index-1];
            memcpy(&entry.value, pD-(index-1)*size, size);
        }
    }
    else {
        entry.key = pK[index];
        memcpy(&entry.value, pD-index*size, size);
    }
}

int BtreeSparseBlock::put(int index, const Entry &entry)
{
    assert(index >= 0 && index <= *nEntries);
    size_t size = getDatumSize();
    Key_t *pK = (Key_t*) payload;
    u8* p = *ph.image + PAGE_SIZE - size;

    // found entry with exact key
    if (index < *nEntries && pK[index] == entry.key) {
        if (isDefault(&entry.value)) {
            del(index);
            return BT_OK;
        }
        else {
            memcpy(p-index*size, &entry.value, size);
            return BT_OVERWRITE;
        }
    }
    // not found
    else {
        if (isDefault(&entry.value))
            return BT_OK;
        u16 &n = *nEntries;
        if (n >= getCapacity()) { // overflow
            OverflowEntry &e = overflowEntries[n-getCapacity()];
            e.index = index;
            e.entry = entry;
            n++;
            return BT_OVERFLOW;
        }
        else {
            // non-overflow
            // NOTE: unsigned type cannot be used for i because
            // nEntries can be 0 and assigning -1 to unsigned
            // causes error.
            int i;

            // shift keys
            for (i = n-1; i >= index; i--) {
                pK[i+1] = pK[i];
            }
            pK[index] = entry.key;
            // shift data
            for (i = n-1; i >= index; i--)
                memcpy(p-(i+1)*size, p-i*size, size);
            //pD[-i-1] = pD[-i];
            memcpy(p-index*size, &entry.value, size);
            n++;
            return BT_OK;
        }
    }
}

void BtreeSparseBlock::truncate(int cutoff)
{
    int i;
    size_t size = getDatumSize();
    Key_t *pK = (Key_t*) payload;
    u8* p = *ph.image + PAGE_SIZE - size;

    upper = pK[cutoff];

    if (*nEntries > getCapacity()) {
        OverflowEntry &e = overflowEntries[0];
        if (e.index < cutoff) {
            // correct upper bound
            upper = pK[cutoff-1];
            // shift stuff and insert the overflown entry
            for (i = cutoff-2; i >= e.index; i--)
                pK[i+1] = pK[i];
            pK[e.index] = e.entry.key;
            for (i = cutoff-2; i >= e.index; i--)
                memcpy(p-(i+1)*size, p-i*size, size);
            memcpy(p-e.index*size, &e.entry.value, size);
        }
    }
    *nEntries = cutoff;
}

BtreeBlock::Iterator* BtreeSparseBlock::getSparseIterator(
    const Key_t &beginsAt, const Key_t &endsBy)
{
    BtreeBlock::Iterator* itor = getSparseIterator();
    itor->setRange(beginsAt, endsBy);
    return itor;
}

BtreeBlock::Iterator* BtreeSparseBlock::getSparseIterator()
{
    BtreeBlock::Iterator* itor = new BtreeSparseBlock::SparseIterator(this);
    return itor;
}

BtreeSparseBlock::SparseIterator::SparseIterator(BtreeSparseBlock *block)
{
    this->block = block;
    index = -1;
    size = block->getSize();
    begin = 0;
    end = size;
}

bool BtreeSparseBlock::SparseIterator::moveNext()
{
    return (++index < end);
}

bool BtreeSparseBlock::SparseIterator::movePrev()
{
    return (--index >= begin);
}

void BtreeSparseBlock::SparseIterator::get(Key_t &k, Value &d)
{
    Entry e;
    block->get(index, e);
    k = e.key;
    d = e.value;
}

void BtreeSparseBlock::SparseIterator::put(const Value &d)
{
    Entry e;
    e.value = d;
    block->put(index, e);
}

void BtreeSparseBlock::SparseIterator::reset()
{
    index = begin - 1;
}

bool BtreeSparseBlock::SparseIterator::setRange(const Key_t &b, const Key_t &e)
{
    if (b < block->getLowerBound() || e > block->getUpperBound()
        || b >=e)
        return false;
    int j;
    block->search(b, begin);
    if (e == block->getUpperBound())
        end = size;
    else
        block->search(e, end);
    reset();
    return true;
}

