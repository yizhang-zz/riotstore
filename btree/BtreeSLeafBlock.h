#ifndef BTREE_SLEAF_BLOCK_H
#define BTREE_SLEAF_BLOCK_H

#include "BtreeSparseBlock.h"

class BtreeSLeafBlock : public BtreeSparseBlock
{
protected:
    PID_t *nextLeaf;
    //static const u16 capacity = ((PAGE_SIZE)-headerSize)/
    //    (sizeof(Key_t)+sizeof(Datum_t));
    
public:
    static const u16 capacity = 5;
    const static u16 headerSize=8;

    BtreeSLeafBlock(PageHandle *pPh, Key_t beginsAt, Key_t endsBy,
                    bool create=true);
    virtual int getDatumSize() { return sizeof(Datum_t); }
    virtual bool isDefault(const void *p) { return *((const Datum_t*)p) == defaultValue; }
    virtual u16 getCapacity() { return capacity; }

    virtual BtreeBlock* copyNew(PageHandle *pPh, Key_t beginsAt, Key_t endsBy)
    {
        return new BtreeSLeafBlock(pPh, beginsAt, endsBy);
    }

    virtual void print(int depth, Btree *tree);
    virtual void setNextLeaf(PID_t pid) { *nextLeaf = pid; }
    virtual BtreeBlock* pack();
};

#endif
