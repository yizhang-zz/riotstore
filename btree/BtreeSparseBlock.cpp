#include "BtreeSparseBlock.h"
#include "BtreeDenseLeafBlock.h"
#include <iostream>

using namespace Btree;

/*
template<class T>
const int SparseBlock<T>::headerSize = 0;

template<>
const int SparseBlock<PID_t>::headerSize = 8;

template<>
const int SparseBlock<Datum_t>::headerSize = 12;

template<class T>
const u16 SparseBlock<T>::capacity = 0;
template<>
const u16 SparseBlock<PID_t>::capacity = 0;
template<>
const u16 SparseBlock<Datum_t>::capacity = 0;
*/

template<>
SparseBlock<PID_t>::SparseBlock(PageHandle ph, Key_t beginsAt, Key_t endsBy, bool create)
	:BlockT<PID_t>(ph, beginsAt, endsBy)
{
	capacity	= config->internalCapacity;
	header = (Header*) ph->getImage();
	// data grows towards low-address direction and cannot pass this boundary
	//boundary	= config->internalHeaderSize+capacity*2; // each offset occupies 2 bytes

	//freeCell	= (u16*)(header+4);
	//dataOffset	= (u16*)(header+6);
	//offsets = (u16*) (header+config->internalHeaderSize);
	if (create) {
		header->flag = kInternal; // flag
		header->nEntries = 0;
		//*freeCell = 0;
		//*dataOffset = PAGE_SIZE;
	}
	pData = ph->getImage() + sizeof(Header);
}

template<>
SparseBlock<Datum_t>::SparseBlock(PageHandle ph, Key_t beginsAt, Key_t endsBy, bool create)
	:BlockT<Datum_t>(ph, beginsAt, endsBy)
{
	capacity	= config->sparseLeafCapacity;
	header = (Header*) ph->getImage();
	// data grows towards low-address direction and cannot pass this boundary
	//boundary	= config->sparseLeafHeaderSize+capacity*2; // each offset occupies 2 bytes

	//freeCell	= (u16*)(header+4);
	//dataOffset	= (u16*)(header+6);
	//offsets = (u16*) (header+config->sparseLeafHeaderSize);
	if (create) {
		header->flag = kSparseLeaf; // flag
		header->nEntries = 0;
		//*freeCell = 0;
		//*dataOffset = PAGE_SIZE;
		header->nextLeaf = INVALID_PID;
	}
	pData = ph->getImage() + sizeof(Header);
}

template<class T>
int SparseBlock<T>::search(Key_t key, int &index) const
{
	// Invariant: if key exists, it must be in [p,q]
	int p = 0;
	int q = this->size() - 1;
	int mid;
	Key_t midKey;

	while (p <= q) {
		mid = (p+q)/2;
		midKey = key_(mid);
		if (midKey == key) {
			index = mid;
			return kOK;
		}
		else if (midKey > key)
			q = mid - 1;
		else
			p = mid + 1;
	}

	index = p;
	return kNotFound;
}

//template<class T>
//int SparseBlock<T>::get(Key_t k, T &v) const

//template<class T>
//int SparseBlock<T>::get(int index, Key_t &k, T &v) const

// This only works for T=Datum_t, i.e., sparse leaf blocks
template<class T>
int SparseBlock<T>::getRange(Key_t beginsAt, Key_t endsBy, T *vals) const
{
	assert(this->type() != Block::kInternal);
	//T *vals = static_cast<T*>(values);
	int index;
	int num = endsBy - beginsAt;
	search(beginsAt, index);
	for (int i=0; i<num; ++i) {
		if (index<(header->nEntries) && key_(index)==beginsAt+i) {
			vals[i] = value_(index);
			++index;
		}
		else
			vals[i] = Block::kDefaultValue;
	}
	return kOK;
}

template<class T>
int SparseBlock<T>::getRange(int beginsAt, int endsBy, Key_t *keys, T *vals) const
{
	//T *vals = static_cast<T*>(values);
	int num = endsBy - beginsAt;
	for (int i=0; i<num; ++i) {
		keys[i] = key_(beginsAt+i);
		vals[i] = value_(beginsAt+i);
	}
	return kOK;
}

template<class T>
int SparseBlock<T>::getRangeWithOverflow(int beginsAt, int endsBy, Key_t *keys, T *vals) const
{
	if (this->isOverflowed &&
            beginsAt <= this->overflow.index && this->overflow.index < endsBy) {
		//T *vals = static_cast<T*>(values);
		for (int i=beginsAt; i<this->overflow.index; ++i) {
			keys[i-beginsAt] = key_(i);
			vals[i-beginsAt] = value_(i);
		}
		keys[this->overflow.index-beginsAt] = this->overflow.key;
		vals[this->overflow.index-beginsAt] = this->overflow.value;
		for (int i=this->overflow.index; i<endsBy-1; ++i) {
			keys[i-beginsAt+1] = key_(i);
			vals[i-beginsAt+1] = value_(i);
		}
		return kOK;
	}
	else if (this->isOverflowed && beginsAt > this->overflow.index)
		return getRange(beginsAt-1, endsBy-1, keys, vals);
	else
		return getRange(beginsAt, endsBy, keys, vals);
}

template<>
int SparseBlock<Datum_t>::batchGet(Key_t beginsAt, Key_t endsBy, std::vector<Entry> &v) const
{
	int index, index_end;
	search(beginsAt, index);
	search(endsBy, index_end);
	while (index < index_end) {
		v.push_back(Entry(key_(index), value_(index)));
		index++;
	}
	return kOK;	
}

template<class T>
int SparseBlock<T>::del(int index)
{
	if (index < 0 || index >= header->nEntries)
		return kOutOfBound;
	if (index < header->nEntries - 1)
		memmove(pData+index*kCellSize, pData+(index+1)*kCellSize, kCellSize*(header->nEntries-index-1));
	--header->nEntries;
	this->pageHandle->markDirty();
	return kOK;
}

template<class T>
int SparseBlock<T>::put(Key_t key, const T &v)
{
	int index;
	search(key, index);
	return put(index, key, v);
}

template<>
int SparseBlock<Datum_t>::put(int index, Key_t key, const Datum_t &v)
{
	assert(key >= this->lower && key < this->upper);
	assert(index >= 0 && index <= header->nEntries);

	// check if overwriting a record
	if (index < header->nEntries && key_(index) == key) {
		if (v == Block::kDefaultValue) // delete existent
			return del(index);
		else  { // overwrite existent
			value_(index) = v;
			pageHandle->markDirty();
			return kOK;
		}
	}
	else if (v == Block::kDefaultValue) // delete non-existent
		return kOK;

	pageHandle->markDirty();

	if (header->nEntries >= this->capacity) { // block full
		this->isOverflowed = true;
		this->overflow.key = key;
		this->overflow.value = v;
		this->overflow.index = index;
		return (canSwitchFormat() ? kSwitchFormat : kOverflow);
	}

	memmove(pData+(index+1)*kCellSize, pData+index*kCellSize, kCellSize*(header->nEntries-index));
	setCell(pData+kCellSize*index, key, v);
	++header->nEntries;
	return kOK;
}

template<>
int SparseBlock<PID_t>::put(int index, Key_t key, const PID_t &v)
{
	assert(key >= this->lower && key < this->upper);
	assert(index >= 0 && index <= header->nEntries);

	pageHandle->markDirty();

	// check if overwriting a record
	if (index < header->nEntries && key_(index) == key) {
		value_(index) = v;
		return kOK;
	}

	if (header->nEntries >= this->capacity) { // block full
		this->isOverflowed = true;
		this->overflow.key = key;
		this->overflow.value = v;
		this->overflow.index = index;
		return kOverflow;
	}

	memmove(pData+(index+1)*kCellSize, pData+index*kCellSize, kCellSize*(header->nEntries-index));
	setCell(pData+kCellSize*index, key, v);
	++header->nEntries;
	return kOK;
}

template<class T>
int SparseBlock<T>::putRangeSorted(Key_t *keys, T *vals, int num, int *numPut)
{
	//u16 new_offsets[this->capacity];
	int p,r=0;
	*numPut = 0;
	int &q = *numPut;
	int count = header->nEntries;

	search(keys[0], p);
	int pp = p;
	char *tmp = new char[(this->capacity-p)*kCellSize];
	char *cur = tmp;

	while (q<num && p<count && header->nEntries<this->capacity) {
		if (key_(p)==keys[q]) {
			setCell(cur, keys[q],vals[q]);
			++p;
			++q;
		}
		else if (key_(p)<keys[q]) {
			setCell(cur, key_(p), value_(p));
			++p;
		}
		else {
			setCell(cur, keys[q], vals[q]);
			++q;
			++(header->nEntries);
		}
		++r;
		cur += kCellSize;
	}
	// Check if existing records have been exhausted during the merge.
	// If so, keep merging new records.
	if (q<num && p==count) {
		while (q<num && header->nEntries<this->capacity) {
			setCell(cur, keys[q], vals[q]);
			++q;
			++(header->nEntries);
			++r;
			cur += kCellSize;
		}
	}

	memcpy(this->pData+pp*kCellSize, tmp, (this->capacity-pp)*kCellSize); 
    delete[] tmp;
	// If we still have new records to be merged, then it must be that
	// we ran out of space. We can actually put one more cause we have room
	// for an overflow entry.
	if (q<num) {
		assert (header->nEntries==this->capacity);
		++q;
		return put(keys[q-1], vals[q-1]);
	}
	this->pageHandle->markDirty();
	return kOK;
}

template<class T>
void SparseBlock<T>::truncate(int sp, Key_t spKey)
{
	this->upper = spKey;
	if (!this->isOverflowed || sp <= this->overflow.index) {
		//freeCells(sp, *this->nEntries);
		this->isOverflowed = false;
		header->nEntries = sp;
	}
	else {
		//freeCells(sp-1, *this->nEntries);
		header->nEntries = sp-1;
		put(this->overflow.index, this->overflow.key, this->overflow.value);
		this->isOverflowed = false;
	}
	this->pageHandle->markDirty();
}

template<class T>
void SparseBlock<T>::print() const
{
	using namespace std;
	cout<<(this->type()==Block::kInternal?"I":"S");
	cout<<"{"<<this->sizeWithOverflow()<<"}";
	cout<<"["<<this->lower<<","<<this->upper<<"] ";
	SparseIterator it(this,0), end(this,this->size());
	for (; it !=end; ++it) {
		cout<<"("<<it->first<<","<<it->second<<")";
	}
	cout<<endl;
}
