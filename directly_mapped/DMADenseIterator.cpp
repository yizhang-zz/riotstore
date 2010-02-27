
#include <string>
#include "DirectlyMappedArray.h"
#include "DMADenseIterator.h"

bool DMADenseIterator::nextBlock() 
{
   if (atLastBlock)
      return false;

   curPid++;
   plb = array->getPageLowerBound(curPid);
   pub = array->getPageUpperBound(curPid);

   if (block) {
       array->releaseBlock(block);
       delete block;
       block = NULL;
   }

   array->readBlock(curPid, &block);

   atLastBlock = (pub >= endsBy);
   if (atLastBlock)
       pub = endsBy;
   return true;
}

DMADenseIterator::DMADenseIterator(Key_t _beginsAt, Key_t _endsBy, DirectlyMappedArray* array) 
{
    block = NULL;
    this->array = array;
    if (array->getLowerBound() > _beginsAt || array->getUpperBound() < _endsBy)
        throw std::string("Iterator range out of array range.");
    setIndexRange(_beginsAt, _endsBy);
}

DMADenseIterator::~DMADenseIterator() 
{
    if (block) {
        array->releaseBlock(block);
        delete block;
    }
}

bool DMADenseIterator::moveNext()
{
    curKey++;
    if (curKey < pub)
        return true;
    return nextBlock();
}

void DMADenseIterator::get(Key_t &k, Datum_t &d)
{
    k = curKey;
    if (!block) {
        d = DirectlyMappedArray::DefaultValue;
    }
    else {
        d = block->get(curKey);
    }
}

void DMADenseIterator::put(const Datum_t &d)
{
    if (!block) {
        array->newBlock(curPid, &block);
    }
    block->put(curKey, d);
}

void DMADenseIterator::reset()
{
   if (block) {
       array->releaseBlock(block);
       delete block;
       block = NULL;
   }

   array->findPage(beginsAt, &curPid);
   plb = array->getPageLowerBound(curPid);
   pub = array->getPageUpperBound(curPid);
   
   array->readBlock(curPid, &block);

   atLastBlock = (pub >= endsBy);

   if (atLastBlock)
       pub = endsBy;
   curKey = (i64)beginsAt-1;
}

bool DMADenseIterator::setIndexRange(Key_t b, Key_t e)
{
   this->beginsAt = b;
   this->endsBy = e;
   reset();
   return true;
}
