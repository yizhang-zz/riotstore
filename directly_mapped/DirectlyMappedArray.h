// -*- mode: c++ -*-

#ifndef DIRECTLY_MAPPED_ARRAY_H
#define DIRECTLY_MAPPED_ARRAY_H

#include "../common/common.h"
#include "../common/block.h"
#include "../lower/BitmapPagedFile.h"
#include "../lower/BufferManager.h"

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
};


/*
	A DirectlyMappedArray stores a one-dimensional array (with index
	starting from 0) in index order in the dense format.  All blocks
	except the last one is full.

	It stores its metadata in its first block.
*/
template<class Key_t, class Datum_t>
class DirectlyMappedArray {
protected:
	BitmapPagedFile *file;
	BufferManager<> *buffer;

	uint32_t numElements;

public:
	// If the file exists, initialize from the file;
	// otherwise create a new one.
	DirectlyMappedArray(const char* fileName, uint32_t numElements);

	Datum_t get(Key_t key) const;
	Datum_t put(Key_t key, Datum_t datum);
};

#endif
