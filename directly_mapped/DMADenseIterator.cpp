
#include <string>
#include "DirectlyMappedArray.h"
#include "DMADenseIterator.h"

bool DMADenseIterator::nextBlockIterator() 
{
   if (atLastBlock)
      return false;

   PID_t pid = block->getPID();
   array->releaseBlock(block);
   delete block;

   if (array->loadBlock(pid+1, &block) != RC_OK) 
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

DMADenseIterator::DMADenseIterator(Key_t _beginsAt, Key_t _endsBy, DirectlyMappedArray* array) 
{
   this->array = array;
   if (array->getLowerBound() > _beginsAt || array->getUpperBound() < _endsBy)
      throw std::string("Iterator range out of array range.");
   this->beginsAt = _beginsAt;
   this->endsBy = _endsBy;

   Key_t upper = _endsBy;
   PID_t pid;
   array->findPage(beginsAt, &pid);
   array->loadBlock(pid, &block);

   atLastBlock = true;
   if (block->getUpperBound() < _endsBy) 
   {
      atLastBlock = false;
      upper = block->getUpperBound();
   }
   iter = block->getIterator(_beginsAt, upper);
}

DMADenseIterator::~DMADenseIterator() 
{
   array->releaseBlock(block);
   delete block;
   delete iter;
}


bool DMADenseIterator::moveNext()
{
   if (iter->moveNext())
      return true;
   return nextBlockIterator();
}

/// unimplemented
bool DMADenseIterator::movePrev()
{
   /*if (iter->movePrev())
      return true;
   return false; // incomplete!*/
}

void DMADenseIterator::get(Key_t &k, Datum_t &d)
{
   iter->get(k, d);
}

void DMADenseIterator::put(const Datum_t &d)
{
   iter->put(d);
}

void DMADenseIterator::reset()
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

