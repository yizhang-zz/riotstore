#ifndef DMA_SPARSE_ITERATOR_H
#define DMA_SPARSE_ITERATOR_H

#include "../common/ArrayInternalIterator.h"
#include "DenseArrayBlock.h"

class DirectlyMappedArray;

class DMASparseIterator : public ArrayInternalIterator
{

   private:
      DirectlyMappedArray *array;
      DenseArrayBlock *block;
      ArrayInternalIterator *iter;
      bool atLastBlock;

      bool nextBlockIterator();
      bool isZero();

   public:
      DMASparseIterator(Key_t _beginsAt, Key_t _endsBy, DirectlyMappedArray* array);
      ~DMASparseIterator();

      bool moveNext();
      bool movePrev();
      void get(Key_t &k, Datum_t &d);
      void put(const Datum_t &d);
      void reset();
};

#endif
