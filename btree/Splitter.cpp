#include "Splitter.h"
#include "BtreeBlock.h"

BtreeBlock* MSplitter::split(BtreeBlock *orig, PageHandle *newHandle)
{
    // orig must be a BtreeSparseBlock
    Key_t beginsAt, endsBy;
    u16 size = orig->getSize();
    u16 mid = size / 2;
    orig->getKey(mid, beginsAt);
    endsBy = orig->getUpperBound();
    BtreeBlock *block = orig->copyNew(newHandle, beginsAt, endsBy);
    Entry e;
    for (u16 k=mid; k<size; k++) {
        orig->get(k, e);
        block->put(k-mid, e);
    }

    orig->truncate(mid);
    return block;
}
