#ifndef DIRECTLY_MAPPED_ARRAY_H
#define DIRECTLY_MAPPED_ARRAY_H

#include "../common/common.h"
#include "../lower/LinearStorage.h"
#include "DenseArrayBlock.h"

struct DirectlyMappedArrayHeader 
{
   uint32_t numElements;
   enum DataType dataType;
    char ch;
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
    u32 numElements;

public:
    static int BufferSize;
    static int getBufferSize();
    const static double DefaultValue = 0.0;
    /// If numElements > 0, create a new array; otherwise read from disk.
    /// Whether file exists is ignored.
    DirectlyMappedArray(const char* fileName, uint32_t numElements);
    virtual ~DirectlyMappedArray();
    
    int get(const Key_t &key, Datum_t &datum);
    int put(const Key_t &key, const Datum_t &datum);
    int batchPut(i64 putCount, const KVPair_t *puts);
    ArrayInternalIterator *createIterator(IteratorType t, Key_t &beginsAt, Key_t &endsBy);
    
    PID_t getPageId(const Key_t &key)
    {
        return key/DenseArrayBlock::CAPACITY + 1;
    }

    void findPage(const Key_t &key, PID_t *pid)
    {
        Key_t CAPACITY = DenseArrayBlock::CAPACITY;
        *pid = key/CAPACITY + 1;
    }
    RC_t readBlock(PID_t pid, DenseArrayBlock** block);
    RC_t newBlock(PID_t pid, DenseArrayBlock** block);
    RC_t readNextBlock(PageHandle ph, DenseArrayBlock** block);
    RC_t releaseBlock(DenseArrayBlock* block, bool dirty=false);
    Key_t getPageLowerBound(PID_t pid);
    Key_t getPageUpperBound(PID_t pid);
    uint32_t getLowerBound();
    uint32_t getUpperBound();
    void *getPageImage(PageHandle ph);
};

#endif
