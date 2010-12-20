#ifndef BTREE_BLOCK_H
#define BTREE_BLOCK_H

#include <string.h>
#include <vector>
#include "../common/common.h"
#include "../common/Iterator.h"
#include "../lower/BufferManager.h"
#include "../lower/PageReplacer.h"
#include <boost/pool/pool.hpp>

namespace Btree
{
//extern boost::pool<> globalInternalBlockPool;
//extern boost::pool<> globalDenseLeafPool;
//extern boost::pool<> globalSparseLeafPool;

	enum Status {
		kOK = 0,
		kOverwrite,
		kOverflow,
		kNotFound,
		kOutOfBound,
		kSwitchFormat
	};

	/**
 * A value stored in a btree block. Can be either a datum or a PID.
 */

/*
  union Value{
    Datum_t datum;
    PID_t pid;
};

struct Entry
{
    Value value;
    Key_t key;
};

struct OverflowEntry : public Entry
{
	// Where the entry should be inserted into.
	// Note that the indices of all other records do NOT change.
	// For example, if the overflow entry has an index of 5,
	// then all existing records with index x >=5 would have a
	// new index x+1 if there were enough space and the overflow
	// entry were inserted into the block.
	u16 index; 
};
*/
//class BTree;

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
	PageHandle pageHandle;
    char	*header;
	u16		capacity;
	Key_t	lower;		// all keys >= lower 
	Key_t	upper;		// all keys < upper 
	// How many entries with non-default values. This never includes the
	// overflow entry, if any.
	u16		*nEntries;
    PID_t	*nextLeaf;
	//OverflowEntry	overflow;
	bool	isOverflowed;

public:
	enum Type {
        kInternal = 0,
        kLeaf = 1,
        kSparseLeaf = 1,
        kDenseLeaf = 3
    };

	//const static int HeaderSize[4];
	//static size_t BlockCapacity[4];
	//static bool capacityInitialized;
	//const static size_t DenseLeafCapacity;
	//  const static size_t SparseLeafCapacity;
	//  const static size_t InternalCapacity;
	//static bool initCapacity();
	//static size_t initCapacity(Type t);

	//int headerSize;	/* makes alignment easier */
	//typedef ::Iterator<Key_t, Value> Iterator;

	//Block(){}
	//static Block * load(BTree *tree, PageHandle ph, Key_t beginsAt, Key_t endsBy);
	//static Block * create(BTree *tree, PageHandle ph, Key_t beginsAt, Key_t endsBy, Type type);

	/* If a key is not found in a dense leaf block, it has a default value
	 * and is omitted. */
	static const Datum_t kDefaultValue = 0.0;
	static inline bool IsDefaultValue(Datum_t x) { return x == kDefaultValue; }
	static Block *create(Type t, PageHandle ph, Key_t beingsAt, Key_t endsBy);
	static Block *create(PageHandle ph, Key_t beingsAt, Key_t endsBy);

	Block(PageHandle ph, Key_t _lower, Key_t _upper)
		:pageHandle(ph),header(ph->getImage()),lower(_lower),upper(_upper),isOverflowed(false)
	{
	}

	virtual ~Block()
	{
	}
	
	virtual int search(Key_t key, int &index) const = 0;
	/**
	 * The specified entry and all after it are truncated.
	 *
	 * @param cutoff Truncate from which entry on
	 */
	virtual void truncate(int pos, Key_t end) = 0;
	virtual size_t valueTypeSize() const { return sizeof(Datum_t); }
	virtual void print() const = 0;

	Key_t getLowerBound() const { return lower; }
	Key_t getUpperBound() const { return upper; }
	void setNextLeaf(PID_t pid) { *nextLeaf = pid; }
	PID_t getNextLeaf() { return *nextLeaf; }
	bool inRange(Key_t key) const { return key >= lower && key < upper; }
	Type type() const { return (Type)(int)*header; }
	bool isDense() { return *((u8*)header) & 2; }
	bool isLeaf() { return *((u8*)header) & 1; }
	u16 size() const { return *nEntries; }
	u16 sizeWithOverflow() const { return (*nEntries)+isOverflowed; }
	void splitTypes(int sp, Key_t spKey, Type *left, Type *right);

	/*
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
		bool setIndexRange(Key_t b, Key_t e);
		bool setRange(const Key_t &b, const Key_t &e);
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
		bool setIndexRange(Key_t b, Key_t e);

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
	*/
};

template<class T>
class BlockT : public Block
{
public:
	BlockT(PageHandle ph, Key_t lower, Key_t upper):Block(ph,lower,upper)
	{
	}

	virtual Key_t key(int index) const = 0;
	virtual T &value(int index) const = 0;

	virtual int get(Key_t key, T &v) const = 0;
	virtual int get(int index, Key_t &key, T &val) const = 0;
	// Returns the beginsAt-th to the endsBy-th (exclusive) records, counting
	// either default-valued or non-default-valued records (in dense format).
	virtual int getRange(Key_t beginsAt, Key_t endsBy, T *values) const = 0;
	// Returns the beginsAt-th to the endsBy-th (exclusive) non-default-valued
	// records (in sparse format).
	virtual int getRange(int beginsAt, int endsBy, Key_t *keys, T *values) const = 0;
	virtual int getRangeWithOverflow(int beginsAt, int endsBy, Key_t *keys, T *values) const = 0;
	virtual int batchGet(Key_t beginsAt, Key_t endsBy, std::vector<Entry> &v) const = 0;

	virtual int put(int index, Key_t key, const T &v) = 0;
	virtual int del(int index) = 0;
	virtual int putRangeSorted(Key_t *keys, T *values, int num, int *numPut) = 0;

#ifndef DISABLE_DENSE_LEAF
	virtual BlockT<T> *switchFormat() = 0;
#endif

	struct OverflowEntry
	{
		int index;
		Key_t key;
		T value;
	} overflow;
};

typedef BlockT<PID_t> PIDBlock;
typedef BlockT<Datum_t> LeafBlock;

}
#endif
