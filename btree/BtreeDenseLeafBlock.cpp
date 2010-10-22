#include "BtreeDenseLeafBlock.h"
#include "BtreeSparseBlock.h"
#include <iostream>

using namespace Btree;

boost::pool<> DenseLeafBlock::memPool(sizeof(DenseLeafBlock));

//u16 DenseLeafBlock::capacity = DenseLeafBlock::initCapacity();

/*
u16 DenseLeafBlock::initCapacity()
{
	u16 cap = (PAGE_SIZE-HeaderSize)/sizeof(Datum_t);
	BtreeConfig *config = BtreeConfig::getInstance();
	if (config) {
		const char *val;
		if ((val=config->get("dleaf_cap"))) {
			cap = atoi(val);
		}
	}
	return cap;
}
*/

DenseLeafBlock::DenseLeafBlock(PageHandle ph, char *image, Key_t beginsAt, Key_t endsBy, bool create)
	:LeafBlock(ph, image, beginsAt, endsBy)
{
	capacity	= config->denseLeafCapacity;
	data		= (Datum_t*)(header+config->denseLeafHeaderSize);
	nEntries	= (u16*)(header+2);
	nextLeaf	= (PID_t*)(header+4);
	headIndex	= (i16*)(header+8);
	tailIndex	= (i16*)(header+10);
	headKey		= (Key_t*)(header+12);
	if (create) {
		*header = kDenseLeaf; // flag
		*nEntries = 0;
		*nextLeaf = INVALID_PID;
		*headIndex = 0;
		*tailIndex = 0;
		*headKey = lower;
	}
}

Status DenseLeafBlock::extendStoredRange(Key_t key) 
{
	int span = getSpan();
	Key_t tailKey = getTailKey();

	if (key < *headKey) {
		if (*headKey-key+span <= capacity) {
			// expand the current segment
			i16 oldIndex = *headIndex;
			*headIndex -= (*headKey - key);
			if (*headIndex < 0) { // wrap-around
				*headIndex += capacity;
				for (i16 i=*headIndex+1; i<capacity; i++)
					data[i] = kDefaultValue;
				for (i16 i=0; i<oldIndex; i++)
					data[i] = kDefaultValue;
			}
			else {
				for (i16 i=*headIndex+1; i<oldIndex; i++)
					data[i] = kDefaultValue;
			}
			*headKey = key;
		}
		else { // cannot fit
			if (canSwitchFormat())
				return kSwitchFormat;
			return kOverflow;
		}
	}
	else if (key >= tailKey) {
		if (key+1-tailKey+span <= capacity) {
			// expand the current segment
			i16 oldIndex = *tailIndex;
			*tailIndex += (key-tailKey+1);
			if (*tailIndex >= capacity) { // wrap-around
				*tailIndex -= capacity;
				for (i16 i=oldIndex; i<capacity; i++)
					data[i] = kDefaultValue;
				for (i16 i=0; i<*tailIndex-1; i++)
					data[i] = kDefaultValue;
			}
			else {
				for (i16 i=oldIndex; i<*tailIndex-1; i++)
					data[i] = kDefaultValue;
			}
		}
		else { // cannot fit
			if (canSwitchFormat())
				return kSwitchFormat;
			return kOverflow;
		}
	}

	return kOK;
}
/*
 * Searches the current block for the specified key. If such key is
 * not stored, kNotFound is returned. Otherwise, the index of the
 * key among all (sorted) records is returned in [out] index.
 */
int DenseLeafBlock::search(Key_t key, int &index) const
{
	assert(key >= lower && key < upper);
	Key_t tailKey = getTailKey();
	if (key >= *headKey && key < tailKey) {
		index = key-*headKey;
		return kOK;
	}
	return kNotFound;
}

//TODO: if key is outside of [head, tail) range, should we return value
//0 and signal kOK or just kNotFound?
int DenseLeafBlock::get(Key_t key, Datum_t &v) const
{
	assert(key >= lower && key < upper);
	Key_t tailKey = getTailKey();
	if (key >= *headKey && key < tailKey) {
		v = value(key-*headKey);
		return kOK;
	}
	return kNotFound;
}

int DenseLeafBlock::get(int index, Key_t &key, Datum_t &v) const
{
	assert(index >= 0 && index < getSpan());
	key = this->key(index);
	v = this->value(index);
	return kOK;
}

int DenseLeafBlock::put(Key_t key, const Datum_t &v, int *index)
{
	assert(key >= lower && key < upper);
	//u16 capacity = getCapacity();

	// key range check
	/*
	if (key >= *headKey && key < tailKey) {
		int index = (*headIndex+(key-*headKey))%capacity;
		if (data[index] == kDefaultValue && v.datum != kDefaultValue)
			*nEntries += 1;
		if (data[index] != kDefaultValue && v.datum == kDefaultValue)
			*nEntries -= 1; // equivalent to deletion
		data[index] = v.datum;
		return kOK;
	}
*/
	// Is block empty?
	if (*nEntries == 0) {
		*headKey = key;
		*headIndex = 0;
		*tailIndex = 1;
		data[0] = v;
		*nEntries = 1;
		*index = 0;
		return kOK;
	}

	Status s;
	switch(s=extendStoredRange(key)) {
	case kOK:
		this->value(key-*headKey) = v;
		*nEntries += 1;
		*index = key-*headKey;
		return kOK;
	case kOverflow:
	case kSwitchFormat:
		isOverflowed = true;
		overflow.key = key;
		overflow.value = v;
		overflow.index = *index = key-*headKey;
		return s;
	default:
		return s;
	}

	/*
	int len = getSpan();
	// insert to the front
	if (key < *headKey) {
		len += *headKey - key;
		if (len <= capacity) {
			// expand the current segment
			i16 oldIndex = *headIndex;
			*headIndex -= (*headKey - key);
			if (*headIndex < 0) { // wrap-around
				*headIndex += capacity;
				for (i16 i=*headIndex+1; i<capacity; i++)
					data[i] = kDefaultValue;
				for (i16 i=0; i<oldIndex; i++)
					data[i] = kDefaultValue;
			}
			else {
				for (i16 i=*headIndex+1; i<oldIndex; i++)
					data[i] = kDefaultValue;
			}

			*headKey = key;
			data[*headIndex] = v.datum;
			*nEntries += 1; // TODO: deletion?
			return kOK;
		}
		else { // overflow; try switching format first
			overflow.key = key;
			overflow.value.datum = v.datum;
			overflow.index = key-*headKey;
			if (*nEntries + 1 <= config->sparseLeafCapacity)
				return kSwitchFormat;
			isOverflowed = true;
			return kOverflow;
		}
		//TODO: store the inserted element elsewhere because
		//we are short of space right now
	}
	// insert to the back
	else { // key >= tailKey
		len += key - tailKey + 1;
		if (len <= capacity) {
			// expand the current segment
			i16 oldIndex = *tailIndex;
			*tailIndex += (key-tailKey+1);
			if (*tailIndex > capacity) { // wrap-around
				*tailIndex -= capacity;
				for (i16 i=oldIndex; i<capacity; i++)
					data[i] = kDefaultValue;
				for (i16 i=0; i<*tailIndex-1; i++)
					data[i] = kDefaultValue;
			}
			else {
				for (i16 i=oldIndex; i<*tailIndex-1; i++)
					data[i] = kDefaultValue;
			}

			data[*tailIndex-1] = v.datum;
			*nEntries += 1; // TODO: deletion?
			return kOK;
		}
		else { // overflow; try switching format first
			overflow.key = key;
			overflow.value.datum = v.datum;
			overflow.index = key-*headKey;
			if (*nEntries + 1 <= config->sparseLeafCapacity)
				return kSwitchFormat;
			isOverflowed = true;
			return kOverflow;
		}
	}
	*/
}

int DenseLeafBlock::getRange(Key_t beginsAt, Key_t endsBy, Datum_t *values) const
{
	//Datum_t *dv = static_cast<Datum_t*>(values);
	DenseIterator it(this, beginsAt-*headKey);
	DenseIterator end(this, endsBy-*headKey);
	for(int i=0; it !=end; ++it,++i) {
		values[i] = it->second;
	}
	return kOK;
}

int DenseLeafBlock::getRange(int beginsAt, int endsBy, Key_t *keys, Datum_t *values) const
{
	//Datum_t *dv = static_cast<Datum_t*>(values);
	SparseIterator it(this, beginsAt);
	SparseIterator end(this, endsBy);
	for(int j=0; j<endsBy-beginsAt && it !=end; ++it,++j) {
		SparseIterator::value_type x = *it; 
		keys[j] = x.first;
		values[j] = x.second;
	}
	return kOK;
}

int DenseLeafBlock::getRangeWithOverflow(int beginsAt, int endsBy, Key_t *keys, Datum_t *values) const
{
	//Datum_t *dv = static_cast<Datum_t*>(values);
	if (isOverflowed) {
		if (overflow.index < 0) {
			if (beginsAt == 0) {
				// fill the first slot
				keys[0] = overflow.key;
				values[0] = overflow.value;
				++beginsAt;
				++keys;
				++values;
			}
			getRange(beginsAt-1,endsBy-1,keys,values);
		}
		else {
			if (endsBy == 1+(*nEntries)) {
				// fill the last slot
				int i = endsBy-beginsAt-1;
				keys[i] = overflow.key;
				values[i] = overflow.value;
				--endsBy;
			}
			getRange(beginsAt,endsBy,keys,values);
		}
	}
	else {
		getRange(beginsAt,endsBy,keys,values);
	}

	return kOK;
}

int DenseLeafBlock::putRangeSorted(Key_t *keys, Datum_t *values, int num, int *numPut)
{
	//Datum_t *dv = static_cast<Datum_t*>(values);
	*numPut = 0;

	int ret;
	for (int i=0; i<num; ++i) {
		//Value v;
		//v.datum = dv[i];
		int index;
		switch(ret=put(keys[i], values[i], &index)) {
		case kOK:
			++(*numPut);
			break;
		default:
			return ret;
		}
	}
	return kOK;
}

void DenseLeafBlock::truncate(int pos, Key_t end)
{
	upper = end;
	if (isOverflowed && overflow.index < 0) {
		--pos;
		if (pos == 0) {
			// the truncated block will only contain the overflow
			// entry, which will be inserted shortly
			*nEntries = 0;
			// to make the while loop below stop
			*tailIndex = *headIndex + 1;
		}
		else
			*tailIndex = (*headIndex + (end-*headKey)) % capacity;
	}
	else if (isOverflowed && overflow.index > 0) {
		if (pos == *nEntries) {
			// *tailIndex remains the same
		}
		else
			*tailIndex = (*headIndex + (end-*headKey)) % capacity;
	}
	else {
			*tailIndex = (*headIndex + (end-*headKey)) % capacity;
	}		
	while (data[(*tailIndex+capacity-1)%capacity] == kDefaultValue)
		--(*tailIndex);
	*nEntries = pos;
	if (isOverflowed) {
		if (overflow.index < 0) {
			int index;
			int ret = put(overflow.key, overflow.value, &index);
			if (ret==kOK) {
				isOverflowed = false;
				++(*nEntries);
				return;
			}
			assert(ret == kSwitchFormat);
		}
		else {
			// must have dropped at least the last (overflow) entry
			isOverflowed = false;
		}
	}
}

LeafBlock *DenseLeafBlock::switchFormat()
{
	//if (t == kSparseLeaf) {
		int num = sizeWithOverflow();
		Key_t *keys = new Key_t[num];
		Datum_t *vals = new Datum_t[num];
		getRangeWithOverflow(0,num,keys,vals);
		SparseLeafBlock *block = new SparseLeafBlock(pageHandle, header,lower,
													 upper, true);
		int numPut;
		block->putRangeSorted(keys,vals,num,&numPut);
		assert(num==numPut);
		delete[] keys;
		delete[] vals;
		return block;
		//}
		//return NULL;
}

void DenseLeafBlock::print() const
{
	using namespace std;
	cout<<"D";
	cout<<"{"<<size()<<"}";
	cout<<"["<<lower<<","<<upper<<"] ";
	SparseIterator it(this, 0), end(this, size());
	for (; it != end; ++it) {
		cout<<"("<<it->first<<","<<it->second<<")";
	}
	cout<<endl;
}
