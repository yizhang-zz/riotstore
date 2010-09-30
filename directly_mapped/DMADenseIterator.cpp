#include <sys/time.h>
#include <string>
#include "DirectlyMappedArray.h"
#include "DMADenseIterator.h"

#ifdef PROFILING
int DMADenseIterator::readCount = 0;
int DMADenseIterator::writeCount = 0;
double DMADenseIterator::accessTime = 0.0;
#endif

bool DMADenseIterator::nextBlock() 
{
   if (atLastBlock)
      return false;

   curPid++;
   plb = array->getPageLowerBound(curPid);
   pub = array->getPageUpperBound(curPid);

   if (block) {
       array->releaseBlock(block, dirty);
       delete block;
       block = NULL;
   }

   array->readBlock(curPid, &block);

   atLastBlock = (pub >= endsBy);
   if (atLastBlock)
       pub = endsBy;
   dirty = false;
   return true;
}

DMADenseIterator::DMADenseIterator(Key_t _beginsAt, Key_t _endsBy, DirectlyMappedArray* array) 
{
    dirty = false;
    block = NULL;
    this->array = array;
    if (array->getLowerBound() > _beginsAt || array->getUpperBound() < _endsBy)
        throw std::string("Iterator range out of array range.");
    setIndexRange(_beginsAt, _endsBy);
}

DMADenseIterator::~DMADenseIterator() 
{
    if (block) {
        array->releaseBlock(block, dirty);
        delete block;
    }
}

bool DMADenseIterator::moveNext()
{
#ifdef PROFILING
    static timeval time1, time2;
    gettimeofday(&time1, NULL);
#endif
    curKey++;
    if (curKey < pub)
        return true;
    bool hasNext = nextBlock();
#ifdef PROFILING
    gettimeofday(&time2, NULL);
    accessTime += time2.tv_sec - time1.tv_sec + (time2.tv_usec - time1.tv_usec)
        / 1000000.0 ;
#endif
    return hasNext;
}

void DMADenseIterator::get(Key_t &k, Datum_t &d)
{
#ifdef PROFILING
    static timeval time1, time2;
    gettimeofday(&time1, NULL);
#endif
    k = curKey;
    if (!block) {
        d = DirectlyMappedArray::DefaultValue;
    }
    else {
        d = block->get(curKey);
    }
#ifdef PROFILING
    gettimeofday(&time2, NULL);
    accessTime += time2.tv_sec - time1.tv_sec + (time2.tv_usec - time1.tv_usec)
        / 1000000.0 ;
    readCount++;
#endif
}

void DMADenseIterator::put(const Datum_t &d)
{
#ifdef PROFILING
    static timeval time1, time2;
    gettimeofday(&time1, NULL);
#endif
    if (!block) {
        array->newBlock(curPid, &block);
    }
    block->put(curKey, d);
    dirty = true;
#ifdef PROFILING
    gettimeofday(&time2, NULL);
    accessTime += time2.tv_sec - time1.tv_sec + (time2.tv_usec - time1.tv_usec)
        / 1000000.0 ;
    writeCount++;
#endif
}

void DMADenseIterator::reset()
{
   if (block) {
       array->releaseBlock(block, dirty);
       delete block;
       block = NULL;
       dirty = false;
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
