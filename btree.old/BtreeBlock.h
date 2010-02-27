#ifndef BTREE_BLOCK_H
#define BTREE_BLOCK_H

#include "../common/common.h"
#include "../common/Iterator.h"
#include <string.h>
#include "../lower/BufferManager.h"
#include "../lower/PageReplacer.h"

#define BT_OK 0
#define BT_OVERWRITE 1
#define BT_OVERFLOW 2
#define BT_NOT_FOUND 3

/**
 * A value stored in a btree block. Can be either a datum or a PID.
 */
union Value
{
    Datum_t datum;
    PID_t pid;
};

/**
 * A key-value pair.
 */
struct Entry
{
    Key_t key;
    Value value;
};

class Btree;

/**
 * A block (node) of a Btree. Structure of a btree block stored on disk looks
 * like:
 *
 * OFFSET	SIZE	DESC
 * 0		1		Flags. bit 1: is leaf, bit 2: is dense. 
 * 1		2		Number of entries.
 * --- the following are optional ---
 * 3		4		Page ID of next leaf; only for leaf blocks.
 * 7        4       Key value at the head position in a dense leaf block.
 * 11       2       Head position in the circular list of a dense leaf block.
 * 13       2       Total # elements occupying a slot in a dense leaf block.
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

class BtreeBlock {
protected:
	u16    *nEntries;	/* how many entries */
	Key_t  lower;		/* all keys >= lower */
	Key_t  upper;		/* all keys < upper */
	PageHandle	ph;
	void   *payload;

    // const static u16 headerSize=12;	/* makes alignment easier */

    BtreeBlock(PageHandle *pPh, Key_t beginsAt, Key_t endsBy,
               bool create=false);
    virtual int del(int index) = 0;

public:
    typedef ::Iterator<Key_t, Value> Iterator;
    
    struct OverflowEntry {
        /// Index of this entry if it is to be placed in the data area
        int index;
        /// The entry holding the key and the value
        Entry entry;
    } overflowEntries[2];

    /* If a key is not found in a dense leaf block, it has a default value
     * and is omitted. */
    const static Datum_t defaultValue = 0.0;
    static inline bool IS_DEFAULT(Datum_t x) { return x == defaultValue; }

    static BtreeBlock *load(PageHandle *pPh, Key_t beginsAt, Key_t endsBy);
    static BtreeBlock *createSameType(BtreeBlock *block, PageHandle *pPh, Key_t beginsAt, Key_t endsBy);
    virtual ~BtreeBlock() {}

    virtual BtreeBlock* copyNew(PageHandle *pPh, Key_t beginsAt, Key_t endsBy) = 0;

//	void syncHeader();

    virtual int search(Key_t key, int &index) = 0;

    virtual int put(Key_t key, const void *p) = 0;

    virtual int get(Key_t key, void *p) = 0;

    // virtual int del(Key_t key) = 0;

    virtual u16 getCapacity() = 0;

    Key_t getLowerBound() { return lower; }
    Key_t getUpperBound() { return upper; }
    u16 & getSize() { return *nEntries; }
    const PageHandle & getPageHandle() { return ph; }
    bool  isSparse() { return !(*ph.image[0] & 2); }
    bool  isLeaf() { return *ph.image[0] & 1; }

    virtual bool  isDefault(const void *p) = 0;
    virtual Key_t getKey(int index) = 0;
    virtual Value getValue(int index) = 0;
    virtual int   get(int index, Entry &e) = 0;
    virtual int   put(int index, const Entry &e) = 0;

    /**
     * The specified entry and all after it are truncated.
     *
     * @param cutoff Truncate from which entry on
     */
    virtual void truncate(int cutoff) = 0;

    virtual BtreeBlock* pack() = 0;

    virtual void print(int depth, Btree *tree) = 0;

    virtual void setNextLeaf(PID_t pid) = 0;
    
    virtual Iterator* getSparseIterator(const Key_t &beingsAt,
                                        const Key_t &endsBy) = 0;    
    virtual Iterator* getDenseIterator(const Key_t &beingsAt,
                                        const Key_t &endsBy) = 0;
    virtual Iterator* getSparseIterator() = 0;
    virtual Iterator* getDenseIterator() = 0;
};

#endif