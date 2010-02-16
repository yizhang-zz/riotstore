#include <string>
#include "DirectlyMappedArray.h"
#include "DMASparseIterator.h"

bool DMASparseIterator::nextBlockIterator() 
{
   if (atLastBlock)
      return false;

   PageHandle ph = block->getPageHandle();
   array->releaseBlock(block);
   delete block;
   int ret = array->loadNextBlock(ph, &block);
   if (ret != RC_OK) 
      return false;

   if (block->getUpperBound() < endsBy) 
   {
      atLastBlock = false;
      delete iter;
      iter = block->getIterator();
   }
   else 
   {
      atLastBlock = true;
      delete iter;
      iter = block->getIterator(block->getLowerBound(), endsBy);
   }
   return true;
}

bool DMASparseIterator::isZero()
{
   Key_t k;
   Datum_t d;
   iter->get(k, d);
   return -0.0000001 < d && d <  0.0000001;
}

DMASparseIterator::DMASparseIterator(Key_t _beginsAt, Key_t _endsBy, DirectlyMappedArray* array) 
{
   this->array = array;
   if (array->getLowerBound() > beginsAt || array->getUpperBound() < endsBy)
      throw std::string("Iterator range out of array range.");
   this->beginsAt = _beginsAt;
   this->endsBy = _endsBy;

   Key_t upper = endsBy;
   PID_t pid;
   array->findPage(beginsAt, &pid);
   array->loadBlock(pid, &block);

   atLastBlock = true;
   if (block->getUpperBound() < endsBy) 
   {
      atLastBlock = false;
      upper = block->getUpperBound();
   }
   iter = block->getIterator(beginsAt, upper);
}

DMASparseIterator::~DMASparseIterator() 
{
   array->releaseBlock(block);
   delete block;
   delete iter;
}


bool DMASparseIterator::moveNext()
{
   do {
      if (!iter->moveNext()) // check current iterator still can move next
         if (!nextBlockIterator()) // check another block after current
            return false;
   } while (isZero()); // check current value is non-zero
   return true;
}

/// UNIMPLEMENTED
bool DMASparseIterator::movePrev()
{
   /*do {
      if (!iter->movePrev()) // check current iterator still can move next
         if (!nextBlockIterator()) // check another block after current
            return false;
   } while (isZero()); // check current value is non-zero
   return true;*/
}

void DMASparseIterator::get(Key_t &k, Datum_t &d)
{
   iter->get(k, d);
}

// cannot put to non-zero values using iterator.....
void DMASparseIterator::put(const Datum_t &d)
{
   iter->put(d);
}

void DMASparseIterator::reset()
{
   array->releaseBlock(block);
   delete block;
   delete iter;

   Key_t upper = endsBy;
   PID_t pid;
   array->findPage(beginsAt, &pid);
   array->loadBlock(pid, &block);

   atLastBlock = true;
   if (block->getUpperBound() < endsBy) 
   {
      atLastBlock = false;
      upper = block->getUpperBound();
   }
   iter = block->getIterator(beginsAt, upper);
}

