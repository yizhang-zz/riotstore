#ifndef BTREE_INT_BLOCK_H
#define BTREE_INT_BLOCK_H

#include "BtreeSparseBlock.h"

class BtreeIntBlock : public BtreeSparseBlock
{
protected:
    // PID_t *rightChild;
    //static const u16 capacity = ((PAGE_SIZE)-headerSize)/
    //    (sizeof(Key_t)+sizeof(PID_t));
    static const u16 capacity = 5;
    
public:
    const static u16 headerSize=4;

    BtreeIntBlock(PageHandle *pPh, Key_t beginsAt, Key_t endsBy,
                    bool create=true);
    virtual int getDatumSize() { return sizeof(PID_t); }
    virtual bool isDefault(void *p) { return false; }
    virtual u16 getCapacity() { return capacity; }
    // virtual void getDatum(int index, void *datum);

    //virtual int put(int index, Key_t *key, void *datum);

    virtual BtreeBlock* copyNew(PageHandle *pPh, Key_t beginsAt, Key_t endsBy)
    {
        return new BtreeIntBlock(pPh, beginsAt, endsBy);
    }

    virtual void print(int depth, Btree *tree);
    virtual void setNextLeaf(PID_t pid) { }
    virtual int search(Key_t key, u16 *index);
    virtual BtreeBlock* pack();
};


#endif
