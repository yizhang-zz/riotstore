#ifndef BTREE_SPARSE_BLOCK_H
#define BTREE_SPARSE_BLOCK_H

#include "BtreeBlock.h"

class BtreeSparseBlock: public BtreeBlock
{
public:

    BtreeSparseBlock(PageHandle *pPh, Key_t beginsAt, Key_t endsBy,
                     bool create=true)
        : BtreeBlock(pPh, beginsAt, endsBy, create)
    {
    }
    
    virtual int search(Key_t key, u16 *index);
    virtual int put(Key_t key, void *p);
    virtual int get(Key_t key, void *p);

    virtual int getDatumSize() = 0;

    virtual Key_t getKey(int index);
    virtual Value getValue(int index);
    virtual int put(int index, Entry &e);
    virtual int get(int index, Entry &e);
    virtual void truncate(int cutoff);

protected:
    virtual int del(int index);
};

#endif
