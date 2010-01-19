#ifndef BTREE_D_LEAF_BLOCk_H
#define BTREE_D_LEAF_BLOCk_H

#include "BtreeBlock.h"
#include "../common/Iterator.h"

class BtreeDLeafBlock : public BtreeBlock
{
public:
    const static u16 headerSize=16;

    virtual int search(Key_t key, int &index);
    virtual int put(Key_t key, const void *p);
    virtual int get(Key_t key, void *p);
    
    static const u16 capacity = 7;
    //static const u16 capacity = ((PAGE_SIZE)-headerSize)/sizeof(Datum_t);

    BtreeDLeafBlock(PageHandle *pPh, Key_t beginsAt, Key_t endsBy,
                    bool create=true);
    
    virtual u16 getCapacity() { return capacity; }
    virtual bool isDefault(const void *p)
    {
        return *((const Datum_t*)p) == defaultValue;
    }
    
    virtual Key_t getKey(int index);
    virtual Value getValue(int index);
    virtual int  get(int index, Entry &e);
    virtual int  put(int index, const Entry &e);
    virtual void truncate(int cutoff);
    virtual BtreeBlock* pack();

    virtual BtreeBlock* copyNew(PageHandle *pPh, Key_t beginsAt, Key_t endsBy)
    {
        return new BtreeDLeafBlock(pPh, beginsAt, endsBy);
    }

    virtual void print(int depth, Btree *tree);

    virtual void setNextLeaf(PID_t pid) { *nextLeaf = pid; }

    virtual Iterator* getSparseIterator(const Key_t &beingsAt,
                                        const Key_t &endsBy);
    virtual Iterator* getSparseIterator();
    virtual Iterator* getDenseIterator(const Key_t &beingsAt,
                                        const Key_t &endsBy);
    virtual Iterator* getDenseIterator();

protected:
    PID_t *nextLeaf;
    u16 *head;
    u16 *nNonZeros;
    Key_t *headKey;

    virtual int del(int index);
    
    /**
     * Finds the position of the element with the given key.
     * 
     * @param key The key to look for
     *
     * @return The position of the element with the given key. -1 if
     * key is less than the first element's key (smallest); size+1 if
     * key is greater than the last element's key (greatest).
     */
    // int find(Key_t key);

public:
    class SparseIterator : public Iterator
    {
    public:
        SparseIterator(BtreeDLeafBlock *block);
        virtual bool moveNext();
        virtual bool movePrev();
        virtual void get(Key_t &k, Value &d);
        virtual void put(const Value &d);
        virtual void reset();
        // virtual ~SparseIterator();
        virtual bool setRange(const Key_t &b, const Key_t &e);
    private:
        BtreeDLeafBlock *block;
        u32 size;
        int index;
        int begin;
        int end;
    };

    class DenseIterator : public Iterator
    {
    public:
        DenseIterator(BtreeDLeafBlock *block);
        virtual bool moveNext();
        virtual bool movePrev();
        virtual void get(Key_t &k, Value &d);
        virtual void put(const Value &d);
        virtual void reset();
        virtual bool setRange(const Key_t &b, const Key_t &e);
    private:
        BtreeDLeafBlock *block;
        Key_t curKey;
    };
};


#endif
