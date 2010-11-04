#pragma once

#include "../common/common.h"
#include "../common/Config.h"
#include "BtreeBlock.h"
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/filter_iterator.hpp>

namespace Btree
{
template<class T>
class SparseBlock : public BlockT<T>
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
    void *operator new(size_t size)
	{
		return memPool.malloc();
	}
	void operator delete(void *p)
	{
		memPool.free(p);
	}
	
	// ctor is specialized
	SparseBlock(PageHandle ph, char *image, Key_t beginsAt, Key_t endsBy, bool create)
		:BlockT<T>(ph, image, beginsAt, endsBy)
	{}
	~SparseBlock()
	{}
	
	int search(Key_t key, int &index) const;


	Key_t key(int i) const
	{
		return key_(i);
	}
	
	T &value(int i) const
	{
		return value_(i);
	}

	int get(Key_t k, T &v) const
	{
		assert(k >= this->lower && k < this->upper);
		int index;
		if (search(k, index) == kOK) {
			v = value_(index);
			return kOK;
		}
		return kNotFound;
	}
	int get(int index, Key_t &k, T &v) const
	{
		assert(index >= 0 && index < *(this->nEntries));
		k = key_(index);
		v = value_(index);
		return kOK;
	}

	int getRange(Key_t beginsAt, Key_t endsBy, T *values) const;
	int getRange(int beginsAt, int endsBy, Key_t *keys, T *values) const;
	int getRangeWithOverflow(int beginsAt, int endsBy, Key_t *keys, T *values) const;
	int put(Key_t key, const T &v, int *index);
	int putRangeSorted(Key_t *keys, T *values, int num, int *numPut);
	void truncate(int sp, Key_t spKey);
	BlockT<T> *switchFormat()
	{
		return NULL;
	}

	//u16 getCapacity() const { return 0; }
	//int getThis->HeaderSize() const { return 0; }
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
	static boost::pool<> memPool;
	static const int kCellSize;
	char *pData;
	//u16 *dataOffset; // where the data section has grown to (from high-address to low-address)
	                 // the next record can be placed s.t. it ends at the current dataOffset
	//u16 *freeCell; // the offset of the first free cell (created by deletions); each free
	               // cell contains a pointer to the next free cell
	//u16 *offsets; // the offset of array containing record offsets
	//u16 boundary; // the offset of the end of the above offset array, which the data section
	              // cannot cross
	/*template<class E>
	void shift(E* array, int length)
	{
		for (int i=length-1; i>=0; i--)
			array[i+1] = array[i];
	}*/
	/*
	char *allocFreeCell()
	{
		char *x = NULL;
		if (boundary <= *dataOffset-sizeof(Key_t)-sizeof(T)) {
			x = this->header+*dataOffset-sizeof(Key_t)-sizeof(T);
			*dataOffset -= sizeof(Key_t)+sizeof(T);
		}
		else {
			x = this->header+*freeCell;
			*freeCell = *(u16*)x;
		}
		return x;
	}*/
	void setCell(char *x, Key_t k, T v)
	{
		*(Key_t*)x = k;
		*(T*)(x+sizeof(k)) = v;
	}
	/*
	u16 offset(Key_t *k)
	{
		return ((char*)k)-this->header;
	}
	*/
	bool canSwitchFormat() const
	{
		Key_t min, max;
		min = key_(0);
		max = key_(*(this->nEntries)-1);
		if (this->overflow.key < min) min = this->overflow.key;
		else if (this->overflow.key > max) max = this->overflow.key;
		return (max-min+1 <= config->denseLeafCapacity);
	}
	/*
	void freeCells(int from_index, int end_index)
	{
		for (int i=from_index; i<end_index; ++i) {
			*(u16*) (this->header+offsets[i]) = *freeCell;
			*freeCell = offsets[i];
		}
	}
	*/
	Key_t &key_(int i) const
	{
		return *(Key_t*)(this->pData+i*kCellSize);
	}

	T &value_(int i) const
	{
		return *(T*)(this->pData+i*kCellSize+sizeof(Key_t));
	}
};

// template specialization declarations

template<>
SparseBlock<PID_t>::SparseBlock(PageHandle ph, char *image, Key_t beginsAt, Key_t endsBy, bool create);

template<>
SparseBlock<Datum_t>::SparseBlock(PageHandle ph, char *image, Key_t beginsAt, Key_t endsBy, bool create);

template<>
BlockT<Datum_t> * SparseBlock<Datum_t>::switchFormat();
/*
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
inline int SparseBlock<PID_t>::getThis->HeaderSize() const
{
	return config->internalThis->HeaderSize;
}

template<>
inline int SparseBlock<Datum_t>::getThis->HeaderSize() const
{
	return config->sparseLeafThis->HeaderSize;
}
*/

typedef SparseBlock<PID_t> InternalBlock;
typedef SparseBlock<Datum_t> SparseLeafBlock;

template<class T>
boost::pool<> SparseBlock<T>::memPool(sizeof(SparseBlock<T>));

template<class T>
const int SparseBlock<T>::kCellSize = sizeof(Key_t) + sizeof(T);
}
//#endif
