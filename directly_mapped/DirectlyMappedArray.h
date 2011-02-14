#ifndef DIRECTLY_MAPPED_ARRAY_H
#define DIRECTLY_MAPPED_ARRAY_H

#include "../common/common.h"
#include "../common/Config.h"
#include "../lower/LinearStorage.h"
#include "DMABlock.h"

/**
 * A DirectlyMappedArray stores a one-dimensional array (with index
 * starting from 0) in index order in the dense format.  All blocks
 * except the last one is full.
 *
 * It stores its metadata in its first block (header). The header includes
 * two pieces of information: 1) The template type of the array. Refer
 * to common.h:DataType. 2) numElements.	
 */
class DirectlyMappedArray : public LinearStorage 
{
public:
    /// If numElements > 0, create a new array; otherwise read from disk.
    /// Whether file exists is ignored.
    DirectlyMappedArray(const char* fileName, Key_t numElements=0);
    virtual ~DirectlyMappedArray();

    virtual int get(const Key_t &key, Datum_t &datum);
    virtual int batchGet(i64 getCount, Entry *gets);
    virtual int batchGet(Key_t beginsAt, Key_t endsBy, std::vector<Entry> &);
    virtual int put(const Key_t &key, const Datum_t &datum);
    virtual int batchPut(i64 putCount, const Entry *puts);
    virtual int batchPut(std::vector<Entry> &);
    virtual void flush();
    virtual StorageType type() const { return DMA; }
    virtual ArrayInternalIterator *createIterator(IteratorType t, Key_t &beginsAt, Key_t &endsBy);
    virtual Key_t upperBound() const { return header->endsBy; }
    virtual size_t nnz() const { return header->nnz; }

    PID_t findPage(Key_t key)
    {
        return key/config->dmaBlockCapacity + 1;
    }

    RC_t readBlock(PID_t pid, DMABlock** block);
    RC_t readOrAllocBlock(PID_t pid, DMABlock** block);
    RC_t newBlock(PID_t pid, DMABlock** block);
    RC_t readNextBlock(PageHandle ph, DMABlock** block);
    //RC_t releaseBlock(DMABlock* block, bool dirty=false);
    Key_t getPageLowerBound(PID_t pid);
    Key_t getPageUpperBound(PID_t pid);
    //void *getPageImage(PageHandle ph);
private:
    PageHandle headerPage;
    struct Header 
    {
        int storageType;
        Key_t endsBy;
        size_t nnz; // number of nonzeros
        //enum DataType dataType;
        //char ch;
    } *header;
    unsigned short putcount;
};

#endif
