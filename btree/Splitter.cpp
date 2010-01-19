#include "Splitter.h"
#include "BtreeBlock.h"
#include "BtreeIntBlock.h"
#include "BtreeDLeafBlock.h"
#include "BtreeSLeafBlock.h"

BtreeBlock* MSplitter::split(BtreeBlock *orig, PageHandle *newHandle)
{
    /*
     * orig's dense/sparse format will not be changed for efficiency
     * reasons. The new node's default format is sparse; but if its
     * range is no larger than a dense node's capacity, then the dense
     * format is used.
     */
    Key_t beginsAt, endsBy; // bounds of new block
    u16 size = orig->getSize();
    u16 sp = size / 2;
    beginsAt = orig->getKey(sp);
    endsBy = orig->getUpperBound();
    BtreeBlock *block;
    if (!orig->isLeaf())
        block = new BtreeIntBlock(newHandle, beginsAt, endsBy);
    else if (orig->isSparse())
        block = new BtreeSLeafBlock(newHandle, beginsAt, endsBy);
    else if (endsBy-beginsAt > BtreeDLeafBlock::capacity)
        block = new BtreeSLeafBlock(newHandle, beginsAt, endsBy);
    else
        block = new BtreeDLeafBlock(newHandle, beginsAt, endsBy);

    // use sparse iterator to move contents to new node
    Entry e;
    BtreeBlock::Iterator *itor = orig->getSparseIterator(beginsAt, endsBy);
    int i = 0;
    while (itor->moveNext()) {
        itor->get(e.key, e.value);
        block->put(i++, e);
    }
    delete itor;

    orig->truncate(sp);
    return block;
}

BtreeBlock* BSplitter::split(BtreeBlock *orig, PageHandle *newHandle)
{
    // start from the middle and find the closest boundary
    Key_t left, right, sp;
    u16 size = orig->getSize();
    if (size % 2 == 0) {
        right = size / 2;
        left = right - 1;
    }
    else {
        left = right = size / 2;
    }
    Entry e1,e2;
    while(true) {
        // test if can split in front of left/right position
        // loop is terminated once a split point is found
        orig->get(right-1, e1);
        orig->get(right, e2);
        if (e1.key/boundary < e2.key/boundary) { // integer comparison
            sp = right;
            break;
        }
        right++;

        orig->get(left-1, e1);
        orig->get(left, e2);
        if (e1.key/boundary < e2.key/boundary) {
            sp = left;
            break;
        }
        left--;
    }

    Key_t beginsAt, endsBy; // bounds of new block
    beginsAt = orig->getKey(sp);
    endsBy = orig->getUpperBound();
    BtreeBlock *block;
    /*
     * orig is guaranteed to be leaf, as internal blocks always use MSplitter.
     */
    if (size-sp > BtreeSLeafBlock::capacity)
        block = new BtreeDLeafBlock(newHandle, beginsAt, endsBy);
    else if (endsBy-beginsAt <= BtreeDLeafBlock::capacity)
        block = new BtreeDLeafBlock(newHandle, beginsAt, endsBy);
    else
        block = new BtreeSLeafBlock(newHandle, beginsAt, endsBy);

    Entry e;
    BtreeBlock::Iterator *itor = orig->getSparseIterator(beginsAt, endsBy);
    int i = 0;
    while (itor->moveNext()) {
        itor->get(e.key, e.value);
        block->put(i++, e);
    }
    delete itor;
    
    orig->truncate(sp);
    return block;
}
