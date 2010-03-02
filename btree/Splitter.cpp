#include "Splitter.h"
#include "BtreeBlock.h"
#include <limits>
using namespace Btree;

Block* MSplitter::split(Block *orig, PageHandle newPh)
{
    /*
     * orig's dense/sparse format will not be changed for efficiency
     * reasons. The new node's default format is sparse; but if its
     * range is no larger than a dense node's capacity, then the dense
     * format is used.
     */
    int sp = orig->getSize() / 2;
    Block *block = orig->split(newPh, sp, orig->getKey(sp));
    return block;
    /*
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
    */
}


Block* BSplitter::split(Block *orig, PageHandle newPh)
{
    // start from the middle and find the closest boundary
    int left, right, sp;
    int size = orig->getSize();
    if (size % 2 == 0) {
        right = size / 2;
        left = right - 1;
    }
    else {
        left = right = size / 2;
    }
    
    while(true) {
        // test if can split in front of left/right position
        // loop is terminated once a split point is found
        Key_t k1, k2;
        k1 = orig->getKey(right-1);
        k2 = orig->getKey(right);
        if (k1/boundary < k2/boundary) { // integer comparison
            sp = right;
            break;
        }
        right++;

        k1 = orig->getKey(left-1);
        k2 = orig->getKey(left);
        if (k1/boundary < k2/boundary) {
            sp = left;
            break;
        }
        left--;
    }

    Key_t mid = (orig->getKey(sp)/boundary)*boundary;
    return orig->split(newPh, sp, mid);

    /*
    // orig is guaranteed to be leaf, as internal blocks always use
    // MSplitter.
    // 
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
    */
}

/*
BtreeBlock* RSplitter::split(BtreeBlock *orig, PageHandle *newHandle)
{
    Key_t beginsAt, endsBy;
    Key_t lower = orig->getLowerBound();
    Key_t upper = orig->getUpperBound();
    int size = orig->getSize();
    double min = std::numeric_limits<double>::max();
    int minind = 0;
    // for each i, try split after the i-th element
    Entry e1, e2;
    for(int i=0; i<size-1; i++) {
        double r1, r2;
        r1 = orig->getKey(i+1) - lower - (i+1);
        if (i+1 <= BtreeSLeafBlock::capacity)
            r1 /= (BtreeSLeafBlock::capacity-i-1);
        else
            r1 /= (BtreeDLeafBlock::capacity-i-1);
        r2 = upper-orig->getKey(i+1)-(size-i-1);
        // todo
    }
    return NULL;
        
}
*/
