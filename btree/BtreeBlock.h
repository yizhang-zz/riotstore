#ifndef BTREE_BLOCK_H
#define BTREE_BLOCK_H

#include "../common/common.h"
#include <string.h>
#include "../lower/BufferManager.h"
#include "../lower/PageReplacer.h"

#define BT_OK 0
#define BT_OVERWRITE 1
#define BT_OVERFLOW 2
#define BT_NOT_FOUND 3

union Value
{
    Datum_t datum;
    PID_t pid;
};

struct Entry
{
    Key_t key;
    Value value;
};

class Btree;

/**
 * Structure of a Btree block: 
 * OFFSET	SIZE	DESC
 * 0		1		Flags. bit 1: is leaf, bit 2: is dense. 
 * 1		2		Number of entries. For internal nodes, this excludes
 * 					the extra child pointer.
 * --- the following are optional ---
 * 3		4		Page ID of next leaf; only for leaf blocks.
 * 3		4		Rightmost child page ID; only for internal blocks.
 * 
 * The lower and upper bounds of a block is not explicitly stored to save
 * space. They can be derived from the keys in the parent block.
 *
 * A dense leaf block contains data that are densely packed. From low
 * address space are entry 0, 1, 2, ...
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
 * and PIDs: lower_bound <= p0 < k0 <= p1 < k1 <= p2 < ... <= pn <
 * upper_bound. There is one more PID than keys; therefore, the rightmost
 * PID is stored in the header as specified above.
 */

class BtreeBlock {
protected:
	u16		*nEntries;	/* how many entries */
	Key_t	lower;		/* all keys >= lower */
	Key_t	upper;		/* all keys < upper */
	//bool	isLeaf;
	//bool	isDense;
	PageHandle	ph;

    const static u16 headerSize=8;	/* should be 7 but 8 makes alignment
									   easier */

	static u16		denseCap;
	static u16		sparseCap;
	static u16		internalCap;

    BtreeBlock(PageHandle *pPh, Key_t beginsAt, Key_t endsBy,
               bool create=false);

public:
    /* If a key is not found in a dense leaf block, it has a default value
     * and is omitted. */
    const static Datum_t defaultValue = 0.0;
    static inline bool IS_DEFAULT(Datum_t x) { return x == defaultValue; }

    static BtreeBlock *load(PageHandle *pPh, Key_t beginsAt, Key_t endsBy);
    
    virtual ~BtreeBlock() {}

    virtual BtreeBlock* copyNew(PageHandle *pPh, Key_t beginsAt, Key_t endsBy) = 0;

//	void syncHeader();

    virtual int search(Key_t key, u16 *index) = 0;

    virtual int put(Key_t key, void *p) = 0;

    virtual int get(Key_t key, void *p) = 0;

    virtual int del(Key_t key) = 0;

    virtual u16 getCapacity() = 0;

    Key_t getLowerBound() { return lower; }
    Key_t getUpperBound() { return upper; }
    u16 & getSize() { return *nEntries; }
    const PageHandle & getPageHandle() { return ph; }

    virtual void getKey(int index, Key_t &key) = 0;
    virtual void getValue(int index, Value &val) = 0;
    virtual int  get(int index, Entry &e) = 0;
    virtual int  put(int index, Entry &e) = 0;

    /**
     * The specified entry and all after it are truncated.
     *
     * @param cutoff Truncate from which entry on
     */
    virtual void truncate(int cutoff) = 0;

    virtual void print(int depth, Btree *tree) = 0;

    virtual void setNextLeaf(PID_t pid) = 0;
};

class BtreeDLeafBlock : public BtreeBlock
{
public:
    virtual int search(Key_t key, u16 *index);
    virtual int put(Key_t key, void *p);
    virtual int get(Key_t key, void *p);
	virtual int del(Key_t key);

protected:
    PID_t *nextLeaf;
    //static const u16 capacity = ((PAGE_SIZE)-headerSize)/sizeof(Datum_t);
    static const u16 capacity = 5;

public:
    BtreeDLeafBlock(PageHandle *pPh, Key_t beginsAt,
                    bool create=true);
    
    virtual u16 getCapacity() { return capacity; }

    virtual void getKey(int index, Key_t &key)
    {
        assert(index >=0 && index < *nEntries);
        key = lower + index;
    }

    virtual void getValue(int index, Value &val)
    {
        assert(index >=0 && index < *nEntries);
        val.datum = ((Datum_t*) (*ph.image + headerSize))[index];
    }

    virtual int get(int index, Entry &e)
    {
        if (index >= 0 && index < *nEntries) {
            e.key = lower + index;
            e.value.datum = ((Datum_t*) (*ph.image + headerSize))[index];
            return BT_OK;
        }
        return BT_NOT_FOUND;
    }

    virtual int put(int index, Entry &e)
    {
        Datum_t *p = (Datum_t*) ((*ph.image)+headerSize);
        p[index] = e.value.datum;
        ++(*nEntries);
        return BT_OK;
        // how about overwrite?
    }

    virtual BtreeBlock* copyNew(PageHandle *pPh, Key_t beginsAt, Key_t endsBy)
    {
        return new BtreeDLeafBlock(pPh, beginsAt);
    }

    virtual void truncate(int cutoff)
    {
        *nEntries = cutoff;
        upper = ((Datum_t*)(*ph.image+headerSize))[cutoff];
    }

    virtual void print(int depth, Btree *tree);

    virtual void setNextLeaf(PID_t pid) { *nextLeaf = pid; }
};

class BtreeSparseBlock: public BtreeBlock
{
public:
    struct OverflowEntry {
		/// Index of this entry if it is to be placed in the data area
		u16 index;
        /// The entry holding the key and the value
        Entry entry;
	} overflowEntries[2];

    BtreeSparseBlock(PageHandle *pPh, Key_t beginsAt, Key_t endsBy,
                     bool create=true)
        : BtreeBlock(pPh, beginsAt, endsBy, create)
    {
    }
    
    virtual int search(Key_t key, u16 *index);
    virtual int put(Key_t key, void *p);
    virtual int get(Key_t key, void *p);
	virtual int del(Key_t key);

    virtual int getDatumSize() = 0;

    virtual void getKey(int index, Key_t &key);
    virtual void getValue(int index, Value &val);
    virtual int put(int index, Entry &e);
    virtual int get(int index, Entry &e);
    virtual void truncate(int cutoff);
};

class BtreeIntBlock : public BtreeSparseBlock
{
protected:
    // PID_t *rightChild;
    //static const u16 capacity = ((PAGE_SIZE)-headerSize)/
    //    (sizeof(Key_t)+sizeof(PID_t));
    static const u16 capacity = 5;
    
public:
    BtreeIntBlock(PageHandle *pPh, Key_t beginsAt, Key_t endsBy,
                    bool create=true);
    virtual int getDatumSize() { return sizeof(PID_t); }

    virtual u16 getCapacity() { return capacity; }
    // virtual void getDatum(int index, void *datum);

    //virtual int put(int index, Key_t *key, void *datum);

    virtual BtreeBlock* copyNew(PageHandle *pPh, Key_t beginsAt, Key_t endsBy)
    {
        return new BtreeIntBlock(pPh, beginsAt, endsBy);
    }

    virtual void print(int depth, Btree *tree);
    virtual void setNextLeaf(PID_t pid) { }
};

class BtreeSLeafBlock : public BtreeSparseBlock
{
protected:
    PID_t *nextLeaf;
    //static const u16 capacity = ((PAGE_SIZE)-headerSize)/
    //    (sizeof(Key_t)+sizeof(Datum_t));
    static const u16 capacity = 5;
    
public:
    BtreeSLeafBlock(PageHandle *pPh, Key_t beginsAt, Key_t endsBy,
                    bool create=true);
    virtual int getDatumSize() { return sizeof(Datum_t); }

    virtual u16 getCapacity() { return capacity; }

    virtual BtreeBlock* copyNew(PageHandle *pPh, Key_t beginsAt, Key_t endsBy)
    {
        return new BtreeSLeafBlock(pPh, beginsAt, endsBy);
    }

    virtual void print(int depth, Btree *tree);
    virtual void setNextLeaf(PID_t pid) { *nextLeaf = pid; }
};

#endif
