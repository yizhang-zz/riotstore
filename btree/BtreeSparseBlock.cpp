#include "BtreeSparseBlock.h"
#include "BtreeDenseLeafBlock.h"
#include <iostream>

using namespace Btree;

//boost::pool<> SparseBlock<Datum_t>::memPool(sizeof(SparseBlock<Datum_t>));
//boost::pool<> SparseBlock<PID_t>::memPool(sizeof(SparseBlock<PID_t>));

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
SparseBlock<PID_t>::SparseBlock(PageHandle ph, char *image, Key_t beginsAt, Key_t endsBy, bool create)
	:BlockT<PID_t>(ph, image, beginsAt, endsBy)
{
	capacity	= config->internalCapacity;
	// data grows towards low-address direction and cannot pass this boundary
	//boundary	= config->internalHeaderSize+capacity*2; // each offset occupies 2 bytes

	nEntries	= (u16*)(header+2);
	//freeCell	= (u16*)(header+4);
	//dataOffset	= (u16*)(header+6);
	//offsets = (u16*) (header+config->internalHeaderSize);
	if (create) {
		*header = kInternal; // flag
		*nEntries = 0;
		//*freeCell = 0;
		//*dataOffset = PAGE_SIZE;
	}
	pData = header+config->internalHeaderSize;
}

template<>
SparseBlock<Datum_t>::SparseBlock(PageHandle ph, char *image, Key_t beginsAt, Key_t endsBy, bool create)
	:BlockT<Datum_t>(ph, image, beginsAt, endsBy)
{
	capacity	= config->sparseLeafCapacity;
	// data grows towards low-address direction and cannot pass this boundary
	//boundary	= config->sparseLeafHeaderSize+capacity*2; // each offset occupies 2 bytes

	nEntries	= (u16*)(header+2);
	//freeCell	= (u16*)(header+4);
	//dataOffset	= (u16*)(header+6);
	//offsets = (u16*) (header+config->sparseLeafHeaderSize);
	nextLeaf	= (PID_t*)(header+8);
	if (create) {
		*header = kSparseLeaf; // flag
		*nEntries = 0;
		//*freeCell = 0;
		//*dataOffset = PAGE_SIZE;
		*nextLeaf = INVALID_PID;
	}
	pData = header+config->sparseLeafHeaderSize;
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
		if (index<*(this->nEntries) && key_(index)==beginsAt+i) {
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
	if (beginsAt <= this->overflow.index && this->overflow.index < endsBy) {
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
	else if (beginsAt > this->overflow.index)
		return getRange(beginsAt-1, endsBy-1, keys, vals);
	else
		return getRange(beginsAt, endsBy, keys, vals);
}

template<class T>
int SparseBlock<T>::put(Key_t key, const T &v, int *index)
{
	assert(key >= this->lower && key < this->upper);
	//int index;

	// check if overwriting a record
	if (search(key, *index) == kOK) {
		//TODO: putting a zero -> deletion
		value_(*index) = v;
		return kOK;
	}

	if (*(this->nEntries) >= this->capacity) { // block full
		this->isOverflowed = true;
		this->overflow.key = key;
		this->overflow.value = v;
		this->overflow.index = *index;
		// sparse internal block
		if (!this->isLeaf()) 
			return kOverflow;
		// sparse leaf block
#ifndef DISABLE_DENSE_LEAF
		return canSwitchFormat()? kSwitchFormat : kOverflow;
#else
		return kOverflow;
#endif
	}

	// new key and enough space
	//char *p = allocFreeCell();
	//setCell(p, key, v);
	//shift(offsets+*index, *this->nEntries-*index);
	//offsets[*index] = p-this->header;
	memmove(pData+(*index+1)*kCellSize, pData+(*index)*kCellSize, kCellSize*(*this->nEntries-*index));
	setCell(pData+kCellSize*(*index), key, v);
	*this->nEntries += 1;
	return kOK;
}

template<class T>
int SparseBlock<T>::putRangeSorted(Key_t *keys, T *vals, int num, int *numPut)
{
	//u16 new_offsets[this->capacity];
	int p,r=0;
	*numPut = 0;
	int &q = *numPut;
	int count = *this->nEntries;

	search(keys[0], p);
	int pp = p;
	char tmp[(this->capacity-p)*kCellSize];
	char *cur = tmp;

	while (q<num && p<count && *this->nEntries<this->capacity) {
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
			++(*this->nEntries);
		}
		++r;
		cur += kCellSize;
	}
	// Check if existing records have been exhausted during the merge.
	// If so, keep merging new records.
	if (q<num && p==count) {
		while (q<num && *this->nEntries<this->capacity) {
			setCell(cur, keys[q], vals[q]);
			++q;
			++(*this->nEntries);
			++r;
			cur += kCellSize;
		}
	}

	memcpy(this->pData+pp*kCellSize, tmp, (this->capacity-pp)*kCellSize); 
	// If we still have new records to be merged, then it must be that
	// we ran out of space. We can actually put one more cause we have room
	// for an overflow entry.
	if (q<num) {
		assert (*this->nEntries==this->capacity);
		++q;
		int indexTemp;
		return put(keys[q-1], vals[q-1], &indexTemp);
	}
	return kOK;
}

template<class T>
void SparseBlock<T>::truncate(int sp, Key_t spKey)
{
	this->upper = spKey;
	if (!this->isOverflowed || sp <= this->overflow.index) {
		//freeCells(sp, *this->nEntries);
		this->isOverflowed = false;
		*this->nEntries = sp;
	}
	else {
		//freeCells(sp-1, *this->nEntries);
		*this->nEntries = sp-1;
		int index;
		put(this->overflow.key, this->overflow.value, &index);
		this->isOverflowed = false;
	}
}

#ifndef DISABLE_DENSE_LEAF
template<>
BlockT<Datum_t> * SparseBlock<Datum_t>::switchFormat()
{
	//if (type==Block::kDenseLeaf) {
		// isOverflowed can be true!
		int num = this->sizeWithOverflow();
		Key_t keys[num];
		Datum_t vals[num];
		getRangeWithOverflow(0,num,keys,vals);
		DenseLeafBlock *block = new DenseLeafBlock(this->pageHandle, this->header,
												   this->lower, this->upper, true);
		int numPut;
		block->putRangeSorted(keys,vals,num,&numPut);
		assert(num==numPut);
		return block;
		//}
		//return NULL;
}
#endif

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
