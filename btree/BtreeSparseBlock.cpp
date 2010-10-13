#include "BtreeSparseBlock.h"
#include "BtreeDenseLeafBlock.h"
#include "BtreeConfig.h"
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
SparseBlock<PID_t>::SparseBlock(char *image, Key_t beginsAt, Key_t endsBy, bool create)
	:Block(beginsAt, endsBy)
{
	//this->tree	= tree;
	//this->ph	= ph;
	//header		= (char*)tree->getPagePacked(ph);
	capacity	= getCapacity();
	header		= image;
	// data grows towards low-address direction and cannot pass this boundary
	boundary	= getHeaderSize()+getCapacity()*2; // each offset occupies 2 bytes

	nEntries	= (u16*)(header+2);
	freeCell	= (u16*)(header+4);
	dataOffset	= (u16*)(header+6);
	offsets = (u16*) (header+getHeaderSize());
	if (create) {
		*header = kInternal; // flag
		*nEntries = 0;
		*freeCell = 0;
		*dataOffset = PAGE_SIZE;
	}
}

template<>
SparseBlock<Datum_t>::SparseBlock(char *image, Key_t beginsAt, Key_t endsBy, bool create)
	:Block(beginsAt, endsBy)
{
	//this->tree	= tree;
	//this->ph	= ph;
	//header		= (char*)tree->getPagePacked(ph);
	capacity	= getCapacity();
	header		= image;
	// data grows towards low-address direction and cannot pass this boundary
	boundary	= getHeaderSize()+getCapacity()*2; // each offset occupies 2 bytes

	nEntries	= (u16*)(header+2);
	freeCell	= (u16*)(header+4);
	dataOffset	= (u16*)(header+6);
	offsets = (u16*) (header+getHeaderSize());
	nextLeaf	= (PID_t*)(header+8);
	if (create) {
		*header = kSparseLeaf; // flag
		*nEntries = 0;
		*freeCell = 0;
		*dataOffset = PAGE_SIZE;
		*nextLeaf = INVALID_PID;
	}
}

template<class T>
int SparseBlock<T>::search(Key_t key, int &index) const
{
	assert(key >= lower && key < upper);
	int size = *nEntries;
	if (size == 0) {
		index = 0;
		return kNotFound;
	}

	int p = 0;
	int q = size - 1;
	int mid;
	Key_t midKey;

	do {
		mid = (p+q)/2;
		midKey = this->key(mid); 
		if (midKey > key)
			q = mid-1;
		else
			p = mid+1;
	} while (p <= q && midKey != key);

	if (midKey == key) {
		index = mid;
		return kOK;
	}
	else {
		index = p;
		return kNotFound;
	}
	return kOK;
}

template<class T>
int SparseBlock<T>::get(Key_t k, Value &v) const
{
	assert(k >= lower && k < upper);
	int index;
	if (search(k, index) == kOK) {
		*(T*)(&v) = value(index);
		return kOK;
	}
	return kNotFound;
}

template<class T>
int SparseBlock<T>::get(int index, Key_t &k, Value &v) const
{
	assert(index >= 0 && index < *nEntries);
	k = key(index);
	*(T*)(&v) = value(index);
	return kOK;
}

// This only works for T=Datum_t, i.e., sparse leaf blocks
template<class T>
int SparseBlock<T>::getRange(Key_t beginsAt, Key_t endsBy, void *values) const
{
	assert(type() != kInternal);
	T *vals = static_cast<T*>(values);
	int index;
	int num = endsBy - beginsAt;
	search(beginsAt, index);
	for (int i=0; i<num; ++i) {
		if (index<*nEntries && key(index)==beginsAt+i) {
			vals[i] = value(index);
			++index;
		}
		else
			vals[i] = kDefaultValue;
	}
	return kOK;
}

template<class T>
int SparseBlock<T>::getRange(int beginsAt, int endsBy, Key_t *keys, void *values) const
{
	T *vals = static_cast<T*>(values);
	int num = endsBy - beginsAt;
	for (int i=0; i<num; ++i) {
		keys[i] = key(beginsAt+i);
		vals[i] = value(beginsAt+i);
	}
	return kOK;
}

template<class T>
int SparseBlock<T>::getRangeWithOverflow(int beginsAt, int endsBy, Key_t *keys, void *values) const
{
	if (beginsAt <= overflow.index && overflow.index < endsBy) {
		T *vals = static_cast<T*>(values);
		for (int i=beginsAt; i<overflow.index; ++i) {
			keys[i-beginsAt] = key(i);
			vals[i-beginsAt] = value(i);
		}
		keys[overflow.index-beginsAt] = overflow.key;
		vals[overflow.index-beginsAt] = *(T*) &overflow.value;
		for (int i=overflow.index; i<endsBy-1; ++i) {
			keys[i-beginsAt+1] = key(i);
			vals[i-beginsAt+1] = value(i);
		}
		return kOK;
	}
	else if (beginsAt > overflow.index)
		return getRange(beginsAt-1, endsBy-1, keys, values);
	else
		return getRange(beginsAt, endsBy, keys, values);
}

template<class T>
int SparseBlock<T>::put(Key_t key, const Value &v)
{
	assert(key >= lower && key < upper);
	int index;

	// check if overwriting a record
	if (search(key, index) == kOK) {
		//TODO: putting a zero -> deletion
		*(T*)(header+offsets[index]+sizeof(Key_t)) = *(T*)&v;
		return kOK;
	}

	if (*nEntries >= getCapacity()) { // block full
		isOverflowed = true;
		overflow.key = key;
		overflow.value = v;
		overflow.index = index;
		// sparse internal block
		if (!isLeaf()) 
			return kOverflow;
		// sparse leaf block
		return canSwitchFormat()? kSwitchFormat : kOverflow;
	}

	// new key and enough space
	char *p = allocFreeCell();
	setCell(p, key, *(T*)&v);
	shift(offsets+index, *nEntries-index);
	offsets[index] = p-header;
	*nEntries += 1;
	return kOK;
}

template<class T>
int SparseBlock<T>::putRangeSorted(Key_t *keys, void *values, int num, int *numPut)
{
	T *data = static_cast<T*>(values);
	u16 new_offsets[capacity];
	int p,r=0;
	*numPut = 0;
	int &q = *numPut;
	int count = *nEntries;

	search(keys[0], p);
	while (q<num && p<count && *nEntries<capacity) {
		if (key(p)==keys[q]) {
			// overwrite
			value(p) = data[q];
			new_offsets[r] = offsets[p];
			++p;
			++q;
		}
		else if (key(p)<keys[q]) {
			new_offsets[r] = offsets[p];
			++p;
		}
		else {
			char *x = allocFreeCell();
			setCell(x, keys[q], data[q]);
			new_offsets[r] = x-header;
			++q;
			++(*nEntries);
		}
		++r;
	}
	// Check if existing records have been exhausted during the merge.
	// If so, keep merging new records.
	if (q<num && p==count) {
		while (q<num && *nEntries<capacity) {
			char *x = allocFreeCell();
			setCell(x, keys[q], data[q]);
			new_offsets[r] = x-header;
			++q;
			++(*nEntries);
			++r;
		}
	}
	memcpy(offsets, new_offsets, sizeof(u16)*size());
	// If we still have new records to be merged, then it must be that
	// we ran out of space. We can actually put one more cause we have room
	// for an overflow entry.
	if (q<num) {
		assert (*nEntries==capacity);
		++q;
		return put(keys[q-1], *(Value*)&data[q-1]);
	}
	return kOK;
}

template<class T>
void SparseBlock<T>::truncate(int sp, Key_t spKey)
{
	upper = spKey;
	if (!isOverflowed || sp <= overflow.index) {
		freeCells(sp, *nEntries);
		isOverflowed = false;
		*nEntries = sp;
	}
	else {
		freeCells(sp-1, *nEntries);
		*nEntries = sp-1;
		put(overflow.key, overflow.value);
		isOverflowed = false;
	}
}

template<class T>
Block * SparseBlock<T>::switchFormat(Type type)
{
	if (this->type()==kSparseLeaf && type==kDenseLeaf) {
		// isOverflowed can be true!
		int num = sizeWithOverflow();
		Key_t keys[num];
		Datum_t vals[num];
		getRangeWithOverflow(0,num,keys,vals);
		DenseLeafBlock *block = new DenseLeafBlock(header,lower,
				upper, true);
		int numPut;
		block->putRangeSorted(keys,vals,num,&numPut);
		assert(num==numPut);
		return block;
	}
	return NULL;
}

template<class T>
void SparseBlock<T>::print() const
{
	using namespace std;
	cout<<(type()==kInternal?"I":"S");
	cout<<"{"<<sizeWithOverflow()<<"}";
	cout<<"["<<lower<<","<<upper<<"] ";
	SparseIterator it(this,0), end(this,size());
	for (; it !=end; ++it) {
		cout<<"("<<it->first<<","<<it->second<<")";
	}
	cout<<endl;
}
