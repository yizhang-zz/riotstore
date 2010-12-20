#include "DMABlock.h"
#include "DirectlyMappedArray.h"
#include "DABIterator.h"

DMABlock::DMABlock(PageHandle ph, Key_t lower, Key_t upper) 
{
    this->ph = ph;
    this->lowerBound = lower;
    this->upperBound = upper;
    this->header = (Header*) (((char*)ph->getImage())+PAGE_SIZE-sizeof(Header));
    this->data = (Datum_t *) ph->getImage();
}

DMABlock::~DMABlock() {}

void DMABlock::init()
{
	for (size_t i=0; i<CAPACITY; ++i)
		data[i] = DefaultValue;
    header->nnz = 0;
}

/// assume key is within range
Datum_t DMABlock::get(Key_t key) const  
{
   return *(data + (key - this->lowerBound));
}

void DMABlock::batchGet(i64 getCount, Entry *gets)
{
    for (i64 i = 0; i < getCount; i++)
    {
        *(gets[i].pdatum) = *(data + (gets[i].key - this->lowerBound));
    }
}

void DMABlock::batchGet(Key_t beginsAt, Key_t endsBy, std::vector<Entry> &v)
{
	int b = std::max(beginsAt, this->lowerBound) - this->lowerBound;
	int e = std::min(endsBy, this->upperBound) - this->lowerBound;
	for (int k = b; k < e; ++k)
		if (data[k] != DefaultValue)
			v.push_back(Entry(k+this->lowerBound, data[k]));
}

/// assume key is within range
int DMABlock::put(Key_t key, Datum_t datum)  
{
    int index = key-lowerBound;
    if (datum == data[index])
        return 0;
    int ret = 0;
    if (datum == DefaultValue && data[index] != DefaultValue)
        ret = -1;
    else if (datum != DefaultValue && data[index] == DefaultValue)
        ret = 1;
    data[index] = datum;
    header->nnz += ret;
    ph->markDirty();
    return ret;
}
      
int DMABlock::batchPut(i64 putCount, const Entry *puts)
{
    int index;
    int ret = 0;
    for (i64 i = 0; i < putCount; i++)
    {
        index = puts[i].key-lowerBound;
        if (puts[i].datum == data[index])
            continue;
        if (puts[i].datum == DefaultValue && data[index] != DefaultValue)
            ret--;
        else if (puts[i].datum != DefaultValue && data[index] == DefaultValue)
            ret++;
        data[index] = puts[i].datum;
    }
    header->nnz += ret;
    ph->markDirty();
    return ret;
}

/// assume beginsAt and endsBy are within upperBound and lowerBound
ArrayInternalIterator *DMABlock::getIterator(Key_t beginsAt, Key_t endsBy) 
{
   return new DABIterator(data+(beginsAt-lowerBound), beginsAt, endsBy);
}

ArrayInternalIterator *DMABlock::getIterator() 
{
   return new DABIterator(data, lowerBound, upperBound);
}

