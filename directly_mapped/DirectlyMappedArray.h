// -*- mode: c++ -*-

#ifndef DIRECTLY_MAPPED_ARRAY_H
#define DIRECTLY_MAPPED_ARRAY_H

#include "../common/common.h"
#include "../common/block.h"
#include "../lower/BitmapPagedFile.h"
#include "../lower/BufferManager.h"
#include <string>

const int BUFFER_SIZE = 100;

template <class _Tp>
class DenseArrayBlockIterator {
	typedef DenseArrayBlockIterator<_Tp> _Self;
	typedef _Tp value_type;
	typedef _Tp* pointer;
	typedef _Tp& reference;
	
private:
	pointer cur;
	pointer end;

public:
	DenseArrayBlockIterator(pointer _cur, pointer _end) {
		cur = _cur;
		end = _end;
	}

	DenseArrayBlockIterator(const _Self& obj) {
		cur = obj.cur;
		end = obj.cur;
	}

	bool next() const {
		if (cur == end)
			return false;
		cur++;
		return true;
	}

	reference operator*() const {
		return *cur;
	}

	// the prefix ++ operator: ++x
	_Self& operator++() {
		cur++;
		return *this;
	}

	// the postfix ++ operator: x++
	_Self operator++(int) {
		_Self tmp = *this;
		cur++;
		return tmp;
	}

	bool operator==(const _Self& x) const {
		return cur == x.cur;
	}

	bool operator!=(const _Self& x) const {
		return cur != x.cur;
	}
};
	
/*
	A DenseArrayBlock does not need a header for the block.  The page
	simply stores a data array.  It stores a continuguous subrange for a
	DirectlyMappedArray.
*/
template<class Datum_t>
class DenseArrayBlock : public Block<Key_t, Datum_t> {
	typedef DenseArrayBlockIterator<Datum_t> iterator;
	
protected:

	// A data array that corresponds to the portion of the
	// DirectlyMappedArray in the index range [beginsAt, endsBy).
	Datum_t *data;

	// - It looks like we don't need a default value for the block, do
	// we?

	// The default value, which has no bearing on how things are stored
	// in the block but affects the behavior of SparseIterator.  Datum_t
	// defaultValue;

public:
	// Initializes the block by reading from a page image.  The index
	// range and the default data value will be passed in---the caller,
	// with the knowledge of the overall DirectlyMappedArray, should
	// know the exact index range that this page is responsible for.
	// The default data value will also be supplied by the caller.
	DenseArrayBlock(PageHandle *ph,
              Key_t lower, Key_t upper);

	~DenseArrayBlock();

	// Datum_t getDefaultValue() const;

	Datum_t get(Key_t key) const; // assume key is within range

	void put(Key_t key, Datum_t datum); // assume key is within range

	iterator getIterator(Key_t beginsAt, Key_t endsBy) {
		return iterator(data+(beginsAt-this->lowerBound), data+(endsBy-this->lowerBound));
	}

	iterator getIterator() {
		return iterator(data, data+this->upperBound-this->lowerBound);
	}
};

template <class _K, class _D>
class DirectlyMappedArray;

template <class _K, class _D>
class DirectlyMappedArrayIterator {
	typedef DirectlyMappedArrayIterator<_K,_D> _Self;
	typedef _D value_type;
	typedef _D* pointer;
	typedef _D& reference;
	
private:
	DirectlyMappedArray<_K,_D>* array;
	DenseArrayBlock<_D>* block;
	DenseArrayBlockIterator<_D> iter;
	_K endsBy;
	bool atLastBlock;

	bool nextBlockIterator() {
		if (atLastBlock)
			return false;

		PID_t pid = block->getPID();
		array->releaseBlock(block);
		delete block;
		
		if (array->loadBlock(pid+1, &block) != RC_SUCCESS) {
			return false;
		}
		if (block->getUpperBound() < endsBy) {
			atLastBlock = false;
			iter = block->getIterator();
		}
		else {
			atLastBlock = true;
			iter = block->getIterator(block->getLowerBound(), endsBy);
		}
		return true;
	}


public:
	DirectlyMappedArrayIterator(_K beginsAt, _K endsBy, DirectlyMappedArray<_K,_D>* array) {
		this->array = array;
		if (array->getLowerBound() > beginsAt || array->getUpperBound() < endsBy)
			throw std::string("Iterator range out of array range.");

		PID_t pid;
		array->findPage(beginsAt, &pid);
		array->loadBlock(pid, &block);
		_K upper = endsBy;
		atLastBlock = true;
		if (block->getUpperBound() < endsBy) {
			atLastBlock = false;
			upper = block->getUpperBound();
		}
		iter = block->getIterator(beginsAt, upper);
	}

	~DirectlyMappedArrayIterator() {
		array->releaseBlock(block);
		delete block;
	}

	bool next() {
		if (iter->next())
			return true;
		else {
			if (nextBlockIterator())
				return iter->next();
			return false;
		}
	}

	reference operator*() const {
		return *iter;
	}

	// the prefix ++ operator: ++x
	_Self& operator++() {
		next();
		return *this;
	}

	// the postfix ++ operator: x++
	_Self operator++(int) {
		_Self tmp = *this;
		next();
		return tmp;
	}

	bool operator==(const _Self& x) const {
		return array == x.array && block->getPID() == x.block->getPID() && iter == x.iter;
	}

	bool operator!=(const _Self& x) const {
		return ! this==x;
	}

};


struct DirectlyMappedArrayHeader {
	uint32_t numElements;
	enum DataType dataType;
};

/*
	A DirectlyMappedArray stores a one-dimensional array (with index
	starting from 0) in index order in the dense format.  All blocks
	except the last one is full.

	It stores its metadata in its first block (header). The header includes
	two pieces of information: 1) The template type of the array. Refer
	to common.h:DataType. 2) numElements.	
*/
template<class Key_t, class Datum_t>
class DirectlyMappedArray {
	typedef DirectlyMappedArrayIterator<Key_t, Datum_t> iterator;

protected:
	BitmapPagedFile *file;
	BufferManager<> *buffer;

	uint32_t numElements;

private:

public:
	// If numElements = 0, create a new array; otherwise read from disk.
	// Whether file exists is ignored.
	DirectlyMappedArray(const char* fileName, uint32_t numElements);

	~DirectlyMappedArray();

	Datum_t get(Key_t key) const;
	void put(Key_t key, Datum_t datum);

	void findPage(Key_t key, PID_t *pid) {
		*pid = key/PAGE_SIZE + 1;
	}

	RC_t loadBlock(PID_t pid, DenseArrayBlock<Datum_t>** block) {
		PageHandle ph;
		ph.pid = pid;
		if (buffer->readPage(ph) == RC_FAILURE)
			return RC_FAILURE;
		*block = new DenseArrayBlock<Datum_t>(ph, PAGE_SIZE*(pid-1), PAGE_SIZE*pid);
		return RC_SUCCESS;
	}

	RC_t releaseBlock(DenseArrayBlock<Datum_t>* block) {
		return buffer->unpinPage(block->ph);
	}

	iterator* getIterator(Key_t beginsAt, Key_t endsBy) {
		return new iterator(beginsAt, endsBy, this);
	}

};

#endif
