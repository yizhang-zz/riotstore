#ifndef DIRECTLY_MAPPED_ARRAY_H
#define DIRECTLY_MAPPED_ARRAY_H

#include "../common/common.h"
#include "../lower/LinearStorage.h"
#include "DenseArrayBlock.h"

const int BUFFER_SIZE = 100;

struct DirectlyMappedArrayHeader 
{
   uint32_t numElements;
   enum DataType dataType;
};

/**
 * A DirectlyMappedArray stores a one-dimensional array (with index
 * starting from 0) in index order in the dense format.  All blocks
 *except the last one is full.
 *
 *  It stores its metadata in its first block (header). The header includes
 *  two pieces of information: 1) The template type of the array. Refer
 * to common.h:DataType. 2) numElements.	
 */
class DirectlyMappedArray : public LinearStorage 
{
   protected:
      uint32_t numElements;

   public:
      /// If numElements > 0, create a new array; otherwise read from disk.
      /// Whether file exists is ignored.
      DirectlyMappedArray(const char* fileName, uint32_t numElements);
      virtual ~DirectlyMappedArray();

      virtual int get(const Key_t &key, Datum_t &datum);
      virtual int put(const Key_t &key, const Datum_t &datum);
      virtual ArrayInternalIterator *createIterator(IteratorType t, Key_t &beginsAt, Key_t &endsBy);

      void findPage(const Key_t &key, PID_t *pid);
      RC_t loadBlock(PID_t pid, DenseArrayBlock** block);
    RC_t loadNextBlock(PageHandle ph, DenseArrayBlock** block);
      RC_t releaseBlock(DenseArrayBlock* block);
      uint32_t getLowerBound();
      uint32_t getUpperBound();
    void *getPageImage(PageHandle ph);

};

#endif
