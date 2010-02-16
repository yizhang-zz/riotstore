#ifndef BTREE_BLOCK_H
#define BTREE_BLOCK_H

#include "../common/common.h"
#include "../common/Iterator.h"
#include <string.h>
#include "../lower/BufferManager.h"
#include "../lower/PageReplacer.h"
#include "SkipList.h"

namespace Btree
{
#define BT_OK 0
#define BT_OVERWRITE 1
#define BT_OVERFLOW 2
#define BT_NOT_FOUND 3

/**
 * A value stored in a btree block. Can be either a datum or a PID.
 */

union Value{
    Datum_t datum;
    PID_t pid;
};

/**
 * A key-value pair.
 */
struct Entry
{
    Value value;
    Key_t key;
};

class BTree;

/**
 * A block (node) of a Btree. Structure of a btree block stored on disk looks
 * like:
 *
 * OFFSET	SIZE	DESC
 * 0		1		Flags. bit 1: is leaf, bit 2: is dense. 
 * 2		2		Number of entries.
 * --- the following are optional ---
 * 4		4		Page ID of next leaf; only for leaf blocks.
 * 8        4       Key value at the head position in a dense leaf block.
 * 12       2       Head position in the circular list of a dense leaf block.
 * 14       2       Total # elements occupying a slot in a dense leaf block.
 * 
 * The lower and upper bounds of a block is not explicitly stored to save
 * space. They can be derived from the keys in the parent block.
 *
 * A dense leaf block contains data that are densely packed. The range of all
 * values that can go into the block can be larger than its capacity, so a
 * dense leaf block can further split. The storage space of a dense leaf block
 * is used as a circular list. The head position marks where the list begins.
 * 
 * A sparse leaf block contains both keys and data. Keys begin right after
 * the header and grow towards the high address end, while data items start
 * from the high-end of the address space of the block and grow downwards.
 * The rational behind this design is better cache performance when
 * searching, because the keys are collocated. For now, there is no prefix
 * compression on keys because long keys often dominate anyway.
 *
 * An internal block contains both keys and PIDs of child blocks. Keys begin
 * right after the header and grow towards the high address end, while PIDs
 * start from the high end and grow downwards. The relationship between keys
 * and PIDs: lower_bound = k0 <= p0 < k1 <= p1 < k2 <= ... <= pn <
 * upper_bound.
 */

class Block {
public:
    BTree *tree;
	u16    *nEntries;	/* how many entries */
	Key_t  lower;		/* all keys >= lower */
	Key_t  upper;		/* all keys < upper */
	PageHandle	ph;
    void   *header;
	void   *payload;
    PID_t *nextLeaf;
    int capacity;

    typedef SkipList<Key_t, Value> List;
    SkipList<Key_t, Value> *list;

public:
    enum Type {
        Internal = 0,
        Leaf = 1,
        SparseLeaf = 1,
        DenseLeaf = 3
    };

    enum {
        DenseHeaderSize = 12,
        SparseHeaderSize = 8,
        InternalHeaderSize = 4
    };

    const static size_t DenseLeafCapacity;
    const static size_t SparseLeafCapacity;
    const static size_t InternalCapacity;
    static size_t initCapacity(Type t);
    
    int headerSize;	/* makes alignment easier */
    typedef ::Iterator<Key_t, Value> Iterator;
    
    Block(BTree *tree, PageHandle pPh, Key_t beginsAt, Key_t endsBy,
          bool create=false, Type type=SparseLeaf);

    /* If a key is not found in a dense leaf block, it has a default value
     * and is omitted. */
    const static Datum_t defaultValue = 0.0;
    static inline bool IS_DEFAULT(Datum_t x) { return x == defaultValue; }

    // static Block *load(PageHandle *pPh, Key_t beginsAt, Key_t endsBy);

    ~Block() {}

    int search(Key_t key, int &index);
    int put(Key_t key, const Value &v);
    int get(Key_t key, Value &v);
    int del(Key_t key);

    u16 getCapacity() { return capacity; }

    Key_t getLowerBound() { return lower; }
    Key_t getUpperBound() { return upper; }
    int getSize() { return list->getSize(); }
    PageHandle getPageHandle() { return ph; }
    bool  isDense() { return *((u8*)header) & 2; }
    bool  isLeaf() { return *((u8*)header) & 1; }

    Key_t getKey(int index);
    Value getValue(int index);
    int   get(int index, Key_t &key, Value &val);
    // int   put(int index, Key_t key, const Value &val) = 0;

    /**
     * The specified entry and all after it are truncated.
     *
     * @param cutoff Truncate from which entry on
     */
    void truncate(int cutoff);
    Block *split(PageHandle newPh, int sp, Key_t spKey);
    void switchToSparse();
    void switchToDense();

    void print(int depth);

    void setNextLeaf(PID_t pid) { *nextLeaf = pid; }
    
    Iterator* getSparseIterator(int beingsAt,
                                int endsBy); 
    Iterator* getDenseIterator(const Key_t &beingsAt,
                               const Key_t &endsBy);
    Iterator* getSparseIterator();
    Iterator* getDenseIterator();

    class SparseIterator : public Iterator
    {
    public:
        SparseIterator(Block *block);
        bool moveNext();
        bool movePrev();
        void get(Key_t &k, Value &d);
        void put(const Value &d);
        void reset();
        bool setIndexRange(int b, int e);
    private:
        Block *block;
        int size;
        int index;
        int begin;
        int end;
    };

    class DenseIterator : public Iterator
    {
    public:
        DenseIterator(Block *block);
        bool moveNext();
        bool movePrev();
        void get(Key_t &k, Value &d);
        void put(const Value &d);
        void reset();
        bool setRange(const Key_t &b, const Key_t &e);
    private:
        Block *block;
        int size;
        int index;
        //int begin;
        //int end;
        Key_t beginKey;
        Key_t endKey;
        Key_t curKey;
        Key_t lastFetchedKey;
        Value lastFetchedVal;
    };
};
}
#endif
