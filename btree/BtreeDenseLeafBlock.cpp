#include "BtreeDenseLeafBlock.h"
#include "BtreeConfig.h"
#include "Btree.h"

using namespace Btree;

u16 DenseLeafBlock::capacity = DenseLeafBlock::initCapacity();

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

DenseLeafBlock::DenseLeafBlock(BTree *tree, PageHandle ph, Key_t beginsAt, Key_t endsBy, bool create)
{
	this->tree	= tree;
	this->ph	= ph;
	lower		= beginsAt;
	upper		= endsBy;
	header		= (char*)tree->getPagePacked(ph);
	data		= (Datum_t*)(header+HeaderSize);
	nEntries	= (u16*)(header+2);
	nextLeaf	= (PID_t*)(header+4);
	headIndex	= (u16*)(header+8);
	tailIndex	= (u16*)(header+10);
	headKey		= (Key_t*)(header+12);
	if (create) {
		*header = DenseLeaf; // flag
		*nEntries = 0;
		*nextLeaf = INVALID_PID;
		*headIndex = 0;
		*tailIndex = 0;
		*headKey = lower;
	}
}

int DenseLeafBlock::search(Key_t key, int &index) const
{
	assert(key >= lower && key < upper);
	Key_t tailKey = getTailKey();
	if (key < *headKey || key >= tailKey) {
		return BT_NOT_FOUND;
	}
	index = (*headIndex+(key-*headKey))%capacity;
	return BT_OK;
}

//TODO: if key is outside of [head, tail) range, should we return value
//0 and signal BT_OK or just BT_NOT_FOUND?
int DenseLeafBlock::get(Key_t key, Value &v) const
{
	assert(key >= lower && key < upper);
	Key_t tailKey = getTailKey();
	if (key < *headKey || key >= tailKey) {
		return BT_NOT_FOUND;
	}
	int index = (*headIndex+(key-*headKey))%capacity;
	v.datum = data[index];
	return BT_OK;
}

int DenseLeafBlock::get(int index, Key_t &key, Value &v) const
{
	assert(index >= 0 && index < capacity);
	v.datum = data[index];
	int diff = index-*headIndex;
	if (diff < 0) diff += capacity;
	key = *headKey + diff;
	return BT_OK;
}

int DenseLeafBlock::put(Key_t key, const Value &v)
{
	assert(key >= lower && key < upper);
	Key_t tailKey = getTailKey();
	if (key >= *headKey && key < tailKey) {
		int index = (*headIndex+(key-*headKey))%capacity;
		if (data[index] == defaultValue && v.datum != defaultValue)
			*nEntries += 1;
		if (data[index] != defaultValue && v.datum == defaultValue)
			*nEntries -= 1; // equivalent to deletion
		data[index] = v.datum;
		return BT_OK;
	}

	int len = (*tailIndex-*headIndex+capacity)%capacity;
	if (key < *headKey) {
		len += *headKey - key;
		if (len <= capacity) {
			// expand the current segment
			u16 oldIndex = *headIndex;
			*headIndex -= (*headKey - key);
			if (*headIndex < 0) { // wrap-around
				*headIndex += capacity;
				for (u16 i=*headIndex+1; i<capacity; i++)
					data[i] = defaultValue;
				for (u16 i=0; i<oldIndex; i++)
					data[i] = defaultValue;
			}
			else {
				for (u16 i=*headIndex+1; i<oldIndex; i++)
					data[i] = defaultValue;
			}

			*headKey = key;
			data[*headIndex] = v.datum;
			*nEntries += 1; // TODO: deletion?
			return BT_OK;
		}
		else { // overflow; try switching format first
		}
	}
	else { // key >= tailKey
		len += key - tailKey + 1;
		if (len <= capacity) {
			// expand the current segment
			u16 oldIndex = *tailIndex;
			*tailIndex += (key-tailKey+1);
			if (*tailIndex >= capacity) { // wrap-around
				*tailIndex -= capacity;
				for (u16 i=oldIndex; i<capacity; i++)
					data[i] = defaultValue;
				for (u16 i=0; i<*tailIndex-1; i++)
					data[i] = defaultValue;
			}
			else {
				for (u16 i=oldIndex; i<*tailIndex-1; i++)
					data[i] = defaultValue;
			}

			data[*tailIndex-1] = v.datum;
			*nEntries += 1; // TODO: deletion?
			return BT_OK;
		}
		else { // overflow; try switching format first
		}
	}
}
