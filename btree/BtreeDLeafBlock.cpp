#include "BtreeDLeafBlock.h"
#include "BtreeSLeafBlock.h"
#include <iostream>


BtreeDLeafBlock::BtreeDLeafBlock(PageHandle *pPh, Key_t beginsAt, Key_t endsBy,
                                 bool create)
    : BtreeBlock(pPh, beginsAt, endsBy, create)
{
    u8 *pData = *ph.image;
    nextLeaf = (PID_t*) (pData+3);
    headKey = (Key_t*) (pData+7);
    head = (u16*) (pData+11);
    nTotal = (u16*) (pData+13);
    payload = pData+15;

    if (create) {
        u8 *flag = *ph.image;
        *flag = 3;
        
        // Init payload to defaultValue
        Datum_t *p = (Datum_t*) payload;
        for(int i=0; i<capacity; i++) {
            p[i] = defaultValue;
        }

        *head = 0;
        *headKey = beginsAt;
        *nTotal = 0;
    }
}

int BtreeDLeafBlock::search(Key_t key, u16 *idx)
{
    assert(key>=lower && key<upper);
    if (*nEntries == 0)
        *idx = 0;
    else
        *idx = key-*headKey;
    return BT_OK;
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
    Datum_t *r = (Datum_t*) pRes;
    if (key < *headKey || key >= *headKey+*nTotal) {
        *r = defaultValue;
        return BT_OK;
    }

    Datum_t *p = (Datum_t*) payload;
    int index = key - *headKey;
    Value v = getValue(index);
    *r = v.datum;
    return BT_OK;
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
    // call index-based put
    int index = key - *headKey;
    Datum_t *p = (Datum_t*) pDatum;
    Entry e;
    e.key = key;
    e.value.datum = *p;
    return put(index, e);

    /*
    Datum_t *p = (Datum_t*) payload;
    u16 offset = key - lower;
    if (IS_DEFAULT(p[offset])) {
        p[offset] = datum;
        ++(*nEntries);
        return BT_OK;
    }
    p[offset] = datum;
    return BT_OVERWRITE;
    */
}

Key_t BtreeDLeafBlock::getKey(int index)
{
    OverflowEntry &e = overflowEntries[0];
    if (*nTotal > capacity) {
        if (e.index < 0) {
            return *headKey+index+e.index;
        }
    }

    return *headKey + index;
}

Value BtreeDLeafBlock::getValue(int index)
{
    Value v;
    OverflowEntry &e = overflowEntries[0];
    if (*nTotal > capacity) {
        // overflow can only happen at either of the two ends
        // overflow at the beginning
        if (e.index < 0) {
            if (index == 0)
                return e.entry.value;
            else if (index < -e.index) {
                v.datum = defaultValue;
                return v;
            }
            else
                index += e.index;
        }
        // overflow at the end
        else {
            if (index == e.index)
                return e.entry.value;
            else if (index >= capacity) {
                v.datum = defaultValue;
                return v;
            }
        }
    }

    Datum_t *p = (Datum_t*) payload;
    v.datum = p[(*head + index + capacity) % capacity];
    return v;
}

/**
 * Gets the entry at the given logical index in the block. Elements are sorted
 * in logical index order.
 *
 * @param index Logical index of the entry.
 * @param e The entry to be returned.
 */
int BtreeDLeafBlock::get(int index, Entry &e)
{
    e.key = getKey(index);
    e.value = getValue(index);
    return BT_OK;
}

int BtreeDLeafBlock::put(int index, Entry &e)
{
    Datum_t *p = (Datum_t*) payload;

    // If new data is defaultValue then it is a deletion
    if (IS_DEFAULT(e.value.datum)) {
        del(index);
        return BT_OK;
    }

    if (*nEntries == 0) {
        // Block is empty
        *nTotal = 1;
        *nEntries = 1;
        *head = 0;
        *headKey = e.key;
        p[0] = e.value.datum;
        return BT_OK;
    }

    if (index >= 0 && index < *nTotal) {
        // Not extending the explicitly stored range of values
        index = (*head + index) % capacity;
        if (IS_DEFAULT(p[index])) {
            *nEntries += 1;
        }
        p[index] = e.value.datum;
        return BT_OK;
    }
    else if (index < 0) {
        // Extending before the beginning of the explicitly stored range

        if ((*nTotal-index) <= capacity) {
            // Current space is enough
            *head = (*head+index+capacity)%capacity;
            (*nEntries)++;
            *nTotal += -index;
            p[*head] = e.value.datum;
            return BT_OK;
        }
        else {
            (*nEntries)++;
            // Scan and reclaim free space if possible
            bool found = false;
            int start = (*head+*nTotal-1)%capacity;
            int end = (*head+index+capacity)%capacity;
            for (int i=start; i>=end; i--) {
                if (!IS_DEFAULT(p[i])) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                // Can move the tail to make room
                *nTotal = capacity;
                *head = end;
                *headKey += index;
                p[*head] = e.value.datum;
                return BT_OK;
            }
            else {
                overflowEntries[0].index = index;
                overflowEntries[0].entry = e;
                *nTotal += -index;
                return BT_OVERFLOW;
            }
        }
    }
    else {
        // Extending after the end of the explicitly stored range

        if (index < capacity) {
            // Current space is enough
            (*nEntries)++;
            *nTotal = index + 1;
            p[(*head+index)%capacity] = e.value.datum;
            return BT_OK;
        }
        else {
            (*nEntries)++;
            // Scan and reclaim free space if possible
            bool found = false;
            int start = *head;
            int end = (*head+index)%capacity;
            for (int i=start; i<=end; i++) {
                if (!IS_DEFAULT(p[i])) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                // can move the head to make room
                *nTotal = capacity;
                *head = (end + 1) % capacity;
                *headKey += end-start+1;
                p[end] = e.value.datum;
                return BT_OK;
            }
            else {
                *nTotal = index+1;
                overflowEntries[0].index = index;
                overflowEntries[0].entry = e;
                return BT_OVERFLOW;
            }
        }
    }
}

/*
 * Deletes an entry with the specified key. If such a key is not stored,
 * just return BT_OK for leaf blocks, or BT_NOT_FOUND for internal blocks. 
 */

int BtreeDLeafBlock::del(int index)
{
    Datum_t *p = (Datum_t*) payload;
    if (index >=0 && index < *nTotal) {
        index = (index+*head)%capacity;
        if (!IS_DEFAULT(p[index])) {
            p[index] = defaultValue;
            --(*nEntries);
        }
    }
    return BT_OK;

    /*
    u16 offset = key - lower;
    if (!IS_DEFAULT(p[offset])) {
        p[offset] = defaultValue;
        --(*nEntries);
    }
    return BT_OK;
    */
}

void BtreeDLeafBlock::truncate(int cutoff)
{
    assert(cutoff <= capacity);
    OverflowEntry &e = overflowEntries[0];
    Datum_t* p = (Datum_t*) payload;
    // Has overflown and needs to move the overflown entry to main storage
    // area
    if (*nTotal > capacity && e.index < 0) {
        // reclaim space for the overflown entry
        for (int i=-1; i>e.index; i--) {
            p[(*head+i+capacity)%capacity] = defaultValue;
        }
        *head = (*head+e.index+capacity)%capacity;
        p[*head] = e.entry.value.datum;
        *headKey += e.index;
    }
    *nTotal = cutoff;
}

void BtreeDLeafBlock::print(int depth, Btree *tree)
{
    using namespace std;
    Datum_t *pD = (Datum_t*) payload;
    for (int i=0; i<depth; i++)
        cout<<"  ";
    cout<<"|_";
    cout<<" D["<<lower<<","<<upper<<")\t{";
    for (int i=0; i<*nTotal; i++)
        cout<<getKey(i)<<", ";
    cout<<"}"<<endl;
}

BtreeBlock* BtreeDLeafBlock::pack()
{
    *nEntries = 0;
    Entry e;
    for (int i=0; i<*nTotal; i++) {
        get(i, e);
        if (!IS_DEFAULT(e.value.datum)) {
            *nEntries += 1;
        }
    }
    
    if (*nEntries < BtreeSLeafBlock::capacity) {
        // convert into a sparse block
        PageImage image;
        PageHandle temp = ph;
        memcpy(image, *ph.image, PAGE_SIZE);
        ph.image = &image;
        u8 *pData = image;
        nEntries = (u16*) (pData+2);
        nextLeaf = (PID_t*) (pData+3);
        headKey = (Key_t*) (pData+7);
        head = (u16*) (pData+11);
        nTotal = (u16*) (pData+13);
        payload = pData+15;

        BtreeSLeafBlock *block = new BtreeSLeafBlock(&temp, lower, upper, true);
        int k = 0;
        for (int i=0; i<*nTotal; i++) {
            get(i, e);
            if (!IS_DEFAULT(e.value.datum)) {
                block->put(k, e);
                k++;
            }
        }
        block->setNextLeaf(*nextLeaf);
        return block;
    }
    else
        return NULL;
}
