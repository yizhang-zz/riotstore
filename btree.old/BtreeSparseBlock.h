#ifndef BTREE_SPARSE_BLOCK_H
#define BTREE_SPARSE_BLOCK_H

#include "BtreeBlock.h"
// #include "../common/Iterator.h"

class BtreeSparseBlock: public BtreeBlock
{
public:

    BtreeSparseBlock(PageHandle *pPh, Key_t beginsAt, Key_t endsBy,
                     bool create=true)
        : BtreeBlock(pPh, beginsAt, endsBy, create)
    {
    }
    
    virtual int search(Key_t key, int &index);
    virtual int put(Key_t key, const void *p);
    virtual int get(Key_t key, void *p);

    virtual int getDatumSize() = 0;

    virtual Key_t getKey(int index);
    virtual Value getValue(int index);
    virtual int put(int index, const Entry &e);
    virtual int get(int index, Entry &e);
    virtual void truncate(int cutoff);

    virtual Iterator* getSparseIterator(const Key_t &beingsAt,
                                        const Key_t &endsBy);
    virtual Iterator* getSparseIterator();
    virtual Iterator* getDenseIterator(const Key_t &beingsAt,
                                        const Key_t &endsBy)
    {
        return NULL;
    }
    virtual Iterator* getDenseIterator()
    {
        return NULL;
    }

protected:
    virtual int del(int index);

public:
    class SparseIterator : public Iterator
    {
    public:
        SparseIterator(BtreeSparseBlock *block);
        virtual bool moveNext();
        virtual bool movePrev();
        virtual void get(Key_t &k, Value &d);
        virtual void put(const Value &d);
        virtual void reset();
        virtual bool setRange(const Key_t &b, const Key_t &e);
    private:
        BtreeSparseBlock *block;
        int size;
        int index;
        int begin;
        int end;
    };
};

#endif
