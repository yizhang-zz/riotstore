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
	
	DenseLeafBlock(PageHandle ph, char *image, Key_t beginsAt, Key_t endsBy, bool create);
	~DenseLeafBlock()
	{
	}
	
	int search(Key_t key, int &index) const;
	int get(Key_t key, Datum_t &v) const;
	int get(int index, Key_t &key, Datum_t &v) const;
	int getRange(Key_t beginsAt, Key_t endsBy, Datum_t *values) const;
	int getRange(int beginsAt, int endsBy, Key_t *keys, Datum_t *values) const;
	int getRangeWithOverflow(int beginsAt, int endsBy, Key_t *keys, Datum_t *values) const;
	int put(Key_t key, const Datum_t &v, int *index);
	int put(int index, Key_t key, const Datum_t &v);
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
						  std::pair<Key_t,Datum_t>,
						  boost::forward_traversal_tag,
						  std::pair<Key_t,Datum_t>
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
		std::pair<Key_t,Datum_t> dereference() const {
			return std::pair<Key_t,Datum_t>(block->key(index),block->value(index));
		}
	};

	class SparseIterator : public boost::iterator_facade<
						  SparseIterator,
						  std::pair<Key_t,Datum_t>,
						  boost::forward_traversal_tag,
						  std::pair<Key_t,Datum_t>
						  >
	{
	public:
		SparseIterator(const DenseLeafBlock *b, int i):block(b),index(i)
		{
			d_index = -1;
			int numNonDefault = 0;
			int span = block->getSpan();
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
		std::pair<Key_t,Datum_t> dereference() const {
			return std::pair<Key_t,Datum_t>(block->key(d_index),block->value(d_index));
		}
	};
	/*
	struct IsNonDefaultValue {
		bool operator()(std::pair<Key_t,Datum_t> x) {
			return x.second!=kDefaultValue;
		}
	};

	typedef boost::filter_iterator<IsNonDefaultValue, DenseIterator>
		_SparseIterator;

	class SparseIterator:public _SparseIterator
	{
	public:
		SparseIterator(const DenseLeafBlock *b, int i):_SparseIterator(predicate, DenseIterator(b,i), DenseIterator(b,b->getSpan()))
		{}
	private:
		static IsNonDefaultValue predicate;
	};
	*/

	/*
	class SparseIterator
	{
	public:
		SparseIterator(DenseLeafBlock *block) {
			this->block = block;
			index = 0;
		}

		SparseIterator& operator++() {
			do {
				++index;
			} while (IsDefaultValue(block->data[(index+*(block->headIndex))%block->capacity]));
			return *this;
		}

		Key_t key() const {
			return index+*(block->headKey);

		}

		Datum_t value() const {
			return block->data[(index+*(block->headIndex))%block->capacity];
		}

	private:
		DenseLeafBlock *block;
		int index;
	};
	*/
private:
	static boost::pool<> memPool;

	i16 *headIndex;
	i16 *tailIndex;
	Key_t *headKey;
	Datum_t *data;
	
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

	int getSpan() const
	{
		int span = *tailIndex - *headIndex;
		if (span == 0 && *nEntries > 0)
			return capacity;
		return span>=0?span:span+capacity;
	}

	bool canSwitchFormat() const
	{
		return (*nEntries + 1) <= config->sparseLeafCapacity;
	}

	Status extendStoredRange(Key_t key) ;
};

}

