#pragma once

#include "../common/common.h"
#include "../common/Config.h"
#include "BtreeBlock.h"
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/filter_iterator.hpp>

namespace Btree
{
class DenseLeafBlock : public LeafBlock
{
public:
	/* Header consists of the following:
	 * Size		Note
	 * 1		flag
	 * 1		reserved
	 * 2		number of non-zero entries stored
	 * 4		page ID of next leaf
	 * 2		head pointer (index in the circular buffer)
	 * 2		tail pointer (index in the circular buffer)
	 * 4		key of the head element 
	 *
	 * We only store a segment among the [lower, upper) range. To facilitate expanding
	 * the segment from both directions, we use a circular buffer to store the segment.
	 * When the head and tail coincide, we decide if the buffer is full or empty by
	 * checking the number of non-zero entries.
	 */
	//const static int HeaderSize = 16;
	//static u16 capacity() { return _capacity; }
	//u16 getCapacity() const { return config->denseLeafCapacity; }
	//int getHeaderSize() const { return config->denseLeafHeaderSize; }
	void *operator new(size_t size)
	{
		return memPool.malloc();
	}
	void operator delete(void *p)
	{
		memPool.free(p);
	}
	
	DenseLeafBlock(PageHandle ph, Key_t beginsAt, Key_t endsBy, bool create);
	~DenseLeafBlock()
	{
	}
	
	int search(Key_t key, int &index) const;
	int get(Key_t key, Datum_t &v) const;
	int get(int index, Key_t &key, Datum_t &v) const;
	int getRange(Key_t beginsAt, Key_t endsBy, Datum_t *values) const;
	int getRange(int beginsAt, int endsBy, Key_t *keys, Datum_t *values) const;
	int getRangeWithOverflow(int beginsAt, int endsBy, Key_t *keys, Datum_t *values) const;
	int batchGet(Key_t beginsAt, Key_t endsBy, std::vector<Entry> &v) const;
	int put(int index, Key_t key, const Datum_t &v);
	int del(int index);
	int putRangeSorted(Key_t *keys, Datum_t *values, int num, int *numPut);
	void truncate(int pos, Key_t end);
	LeafBlock *switchFormat();
	void print() const;

	Key_t key(int index) const { return key_(index); }
	Key_t key_(int index) const { return *headKey + index; }
	Datum_t &value(int index) const { return value_(index); }
	Datum_t &value_(int index) const
	{
		int i = index + *headIndex;
		if (i >= capacity)
			i -= capacity;
		return data[i];
	}

	class DenseIterator : public boost::iterator_facade<
						  DenseIterator,
						  Entry,
						  boost::forward_traversal_tag,
						  Entry
						  >
	{
	public:
		DenseIterator(const DenseLeafBlock *b, int i):block(b),index(i)
		{}

	private:
		const DenseLeafBlock *block;
		int index;
		friend class boost::iterator_core_access;
		void increment() {
			++index;
		}
		bool equal(DenseIterator const & other) const {
			return index == other.index;
		}
		Entry dereference() const {
			return Entry(block->key_(index),block->value_(index));
		}
	};

	class SparseIterator : public boost::iterator_facade<
						  SparseIterator,
						  Entry,
						  boost::forward_traversal_tag,
						  Entry
						  >
	{
	public:
		SparseIterator(const DenseLeafBlock *b, int i):block(b),index(i)
		{
			d_index = -1;
			int numNonDefault = 0;
			int span = block->tailKey - *block->headKey;
			while (d_index < span && numNonDefault < index+1) {
				++d_index;
				if (block->value(d_index) != kDefaultValue)
					++numNonDefault;
			}
		}

	private:
		const DenseLeafBlock *block;
		int index;
		int d_index;
		friend class boost::iterator_core_access;
		void increment() {
			++index;
			do {
				++d_index;
			} while (block->value(d_index) == kDefaultValue);
		}
		bool equal(SparseIterator const & other) const {
			return index == other.index;
		}
		Entry dereference() const {
			return Entry(block->key(d_index),block->value(d_index));
		}
	};

	// The two KeyIterator classes below work even if the block's lower and
	// upper bounds are not set.  This makes it possible to traverse leaf nodes
	// by following nextLeaf pointers, without having access to parent nodes
	// (from which the bounds are obtained).
	
	class DenseKeyIterator : public boost::iterator_facade<
						  DenseKeyIterator,
						  Entry,
						  boost::forward_traversal_tag,
						  Entry
						  >
	{
	public:
		DenseKeyIterator(const DenseLeafBlock *b, Key_t k):block(b)
		{
			if (k > block->upper) k = block->upper;
			else if (k < block->lower) k = block->lower;
		}

	private:
		const DenseLeafBlock *block;
		Key_t key;
		friend class boost::iterator_core_access;
		void increment() {
			++key;
		}
		bool equal(DenseKeyIterator const & other) const {
			return key == other.key;
		}
		Entry dereference() const {
			if (key < *block->headKey || key >= block->tailKey)
				return Entry(key, kDefaultValue);
			return Entry(key, block->value_(key-*block->headKey));
		}
	};

	class SparseKeyIterator : public boost::iterator_facade<
						  SparseKeyIterator,
						  Entry,
						  boost::forward_traversal_tag,
						  Entry
						  >
	{
	public:
		SparseKeyIterator(const DenseLeafBlock *b, Key_t k):block(b)
		{
			if (k > block->tailKey) k = block->tailKey;
			if (k < *block->headKey) k = *block->headKey;
			index = k - *block->headKey;
			while (k < block->tailKey && block->value_(index) == Block::kDefaultValue)
				k++, index++;
		}

	private:
		const DenseLeafBlock *block;
		int index;
		friend class boost::iterator_core_access;
		void increment() {
			do {
				++index;
			} while (block->key_(index) < block->tailKey 
					&& block->value_(index) == Block::kDefaultValue);
		}
		bool equal(SparseKeyIterator const & other) const {
			return index == other.index;
		}
		Entry dereference() const {
			return Entry(block->key_(index),block->value_(index));
		}
	};

private:
	static boost::pool<> memPool;

	i16 *headIndex; // inclusive
	i16 *tailIndex;	// exclusive
	Key_t *headKey;
	Key_t tailKey;
	Datum_t *data;
	
	/*
	Key_t getTailKey() const
	{
		if (*tailIndex > *headIndex) {
			return *headKey+(*tailIndex-*headIndex);
		}
		else if (*tailIndex < *headIndex) {
			return *headKey+(capacity+*tailIndex-*headIndex);
		}
		else {
			if (*nEntries)
				return *headKey+capacity;
			else
				return *headKey;
		}
	}
	*/

	int put(Key_t key, const Datum_t &v);
	/*
	int getSpan() const
	{
		int span = *tailIndex - *headIndex;
		if (span == 0 && *nEntries > 0)
			return capacity;
		return span>=0?span:span+capacity;
	}
	*/
	bool canSwitchFormat() const
	{
		return (*nEntries + 1) <= config->sparseLeafCapacity;
	}

	Status extendStoredRange(Key_t key) ;
};

}

