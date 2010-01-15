#ifndef BTREE_D_LEAF_BLOCk_H
#define BTREE_D_LEAF_BLOCk_H

#include "BtreeBlock.h"


class BtreeDLeafBlock : public BtreeBlock
{
public:
    const static u16 headerSize=16;

    virtual int search(Key_t key, u16 *index);
    virtual int put(Key_t key, void *p);
    virtual int get(Key_t key, void *p);
    
    static const u16 capacity = 7;
    //static const u16 capacity = ((PAGE_SIZE)-headerSize)/sizeof(Datum_t);

    BtreeDLeafBlock(PageHandle *pPh, Key_t beginsAt, Key_t endsBy,
                    bool create=true);
    
    virtual u16 getCapacity() { return capacity; }
    virtual bool isDefault(void *p) { return *((Datum_t*)p) == defaultValue; }
    virtual Key_t getKey(int index);
    virtual Value getValue(int index);
    virtual int  get(int index, Entry &e);
    virtual int  put(int index, Entry &e);
    virtual void truncate(int cutoff);
    virtual BtreeBlock* pack();

    virtual BtreeBlock* copyNew(PageHandle *pPh, Key_t beginsAt, Key_t endsBy)
    {
        return new BtreeDLeafBlock(pPh, beginsAt, endsBy);
    }

    virtual void print(int depth, Btree *tree);

    virtual void setNextLeaf(PID_t pid) { *nextLeaf = pid; }

protected:
    PID_t *nextLeaf;
    u16 *head;
    u16 *nTotal;
    Key_t *headKey;

    virtual int del(int index);
};


#endif
