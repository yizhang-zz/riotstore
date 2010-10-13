#pragma once

#include "../common/common.h"
#include "BtreeBlock.h"
#include "BtreeConfig.h"
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/filter_iterator.hpp>

namespace Btree
{
template<class T>
class SparseBlock : public Block
{
public:
	/* Header of a sparse leaf block consists of the following:
	 * Size		Note
	 * 1		flag
	 * 1		reserved
	 * 2		number of non-zero entries stored
	 * 2		offset of first free space
	 * 2		offset of first byte of cell content area
	 * 4		page ID of next leaf
	 *
	 * Header of a sparse internal block consists of the following:
	 * Size		Note
	 * 1		flag
	 * 1		reserved
	 * 2		number of non-zero entries stored
	 * 2		offset of first free space
	 * 2		offset of first byte of cell content area
	 */
	//const static int headerSize;
	//const static u16 capacity;

	SparseBlock(char *image, Key_t beginsAt, Key_t endsBy, bool create) {}
	int search(Key_t key, int &index) const;
	int get(Key_t key, Value &v) const;
	int get(int index, Key_t &key, Value &v) const;
	int getRange(Key_t beginsAt, Key_t endsBy, void *values) const;
	int getRange(int beginsAt, int endsBy, Key_t *keys, void *values) const;
	int getRangeWithOverflow(int beginsAt, int endsBy, Key_t *keys, void *values) const;
	int put(Key_t key, const Value &v);
	int putRangeSorted(Key_t *keys, void *values, int num, int *numPut);
	void truncate(int sp, Key_t spKey);
	Block *switchFormat(Type type);

	u16 getCapacity() const { return 0; }
	int getHeaderSize() const { return 0; }
	size_t valueTypeSize() const { return sizeof(T); }
	void print() const;

	// iterator
	class SparseIterator : public boost::iterator_facade<
						  SparseIterator,
						  std::pair<Key_t,T>,
						  boost::forward_traversal_tag,
						  std::pair<Key_t,T>
						  >
	{
	public:
		SparseIterator(const SparseBlock<T> *b, int i):block(b),index(i)
	   	{ }
	private:
		friend class boost::iterator_core_access;
		void increment() {
			++index;
		}
		bool equal(SparseIterator const & other) const {
			return index == other.index;
		}
		std::pair<Key_t,T> dereference() const {
			return std::pair<Key_t,Datum_t>(block->key(index),block->value(index));
		}

		const SparseBlock<T> *block;
		int index;
	};
private:
	//Datum_t *data;
	//static u16 initCapacity();
	//static u16 initHeaderSize();
	u16 *dataOffset; // where the data section has grown to (from high-address to low-address)
	                 // the next record can be placed s.t. it ends at the current dataOffset
	u16 *freeCell; // the offset of the first free cell (created by deletions); each free
	               // cell contains a pointer to the next free cell
	u16 *offsets; // the offset of array containing record offsets
	u16 boundary; // the offset of the end of the above offset array, which the data section
	              // cannot cross
	template<class E>
	void shift(E* array, int length)
	{
		for (int i=length-1; i>=0; i--)
			array[i+1] = array[i];
	}
	char *allocFreeCell()
	{
		char *x = NULL;
		if (boundary <= *dataOffset-sizeof(Key_t)-sizeof(T)) {
			x = header+*dataOffset-sizeof(Key_t)-sizeof(T);
			*dataOffset -= sizeof(Key_t)+sizeof(T);
		}
		else {
			x = header+*freeCell;
			*freeCell = *(u16*)x;
		}
		return x;
	}
	void setCell(char *x, Key_t k, T v)
	{
		*(Key_t*)x = k;
		*(T*)(x+sizeof(k)) = v;
	}
	Key_t &key(int i) const
	{
		return *(Key_t*) (header+offsets[i]);
	}
	T &value(int i) const
	{
		return *(T*)(header+offsets[i]+sizeof(Key_t));
	}
	u16 offset(Key_t *k)
	{
		return ((char*)k)-header;
	}
	bool canSwitchFormat() const
	{
		Key_t min, max;
		Value val;
		get(0, min, val);
		get(*nEntries-1, max, val);
		if (overflow.key < min) min = overflow.key;
		else if (overflow.key > max) max = overflow.key;
		return (max-min+1 <= config->denseLeafCapacity);
	}
	void freeCells(int from_index, int end_index)
	{
		for (int i=from_index; i<end_index; ++i) {
			*(u16*) (header+offsets[i]) = *freeCell;
			*freeCell = offsets[i];
		}
	}
};

// template specialization declarations

template<>
SparseBlock<PID_t>::SparseBlock(char *image, Key_t beginsAt, Key_t endsBy, bool create);

template<>
SparseBlock<Datum_t>::SparseBlock(char *image, Key_t beginsAt, Key_t endsBy, bool create);

template<>
inline u16 SparseBlock<PID_t>::getCapacity() const
{
	return config->internalCapacity;
}

template<>
inline u16 SparseBlock<Datum_t>::getCapacity() const
{
	return config->sparseLeafCapacity;
}

template<>
inline int SparseBlock<PID_t>::getHeaderSize() const
{
	return config->internalHeaderSize;
}

template<>
inline int SparseBlock<Datum_t>::getHeaderSize() const
{
	return config->sparseLeafHeaderSize;
}


typedef SparseBlock<PID_t> InternalBlock;
typedef SparseBlock<Datum_t> SparseLeafBlock;

}
//#endif
