#include "DenseArrayBlock.h"
#include "DirectlyMappedArray.h"
#include "DABIterator.h"

DenseArrayBlock::DenseArrayBlock(DirectlyMappedArray *arr,
                                 PageHandle ph, Key_t lower, Key_t upper) 
{
    this->array = arr;
   this->ph = ph;
   this->lowerBound = lower;
   this->upperBound = upper;
   this->data = (Datum_t *) array->getPageImage(ph);
}

DenseArrayBlock::~DenseArrayBlock() {}

// Datum_t getDefaultValue() const;
bool DenseArrayBlock::isFull() const {}

/// assume key is within range
Datum_t DenseArrayBlock::get(Key_t key) const  
{
   return *(data + (key - this->lowerBound));
}

/// assume key is within range
void DenseArrayBlock::put(Key_t key, Datum_t datum)  
{
   *(data + (key - this->lowerBound)) = datum;
}

/// assume beginsAt and endsBy are within upperBound and lowerBound
ArrayInternalIterator *DenseArrayBlock::getIterator(Key_t beginsAt, Key_t endsBy) 
{
   return new DABIterator(data + (beginsAt - lowerBound), beginsAt, endsBy);
}

ArrayInternalIterator *DenseArrayBlock::getIterator() 
{
   return new DABIterator(data, this->lowerBound, this->upperBound);
}

