#ifndef DMA_DENSE_ITERATOR_H
#define DMA_DENSE_ITERATOR_H

#include "../common/ArrayInternalIterator.h"
#include "DenseArrayBlock.h"

class DirectlyMappedArray;

class DMADenseIterator : public ArrayInternalIterator
{

private:
    DirectlyMappedArray *array;
    DenseArrayBlock *block;
    ArrayInternalIterator *iter;
    bool atLastBlock;

    bool nextBlockIterator();
    Key_t beginsAt;
    Key_t endsBy;

public:
    DMADenseIterator(Key_t _beginsAt, Key_t _endsBy, DirectlyMappedArray* array);
    ~DMADenseIterator();

    bool moveNext();
    bool movePrev();
    void get(Key_t &k, Datum_t &d);
    void put(const Datum_t &d);
    void reset();
    bool setRange(const Key_t &b, const Key_t &e)
    {
        throw("not implemented");
        return false;
    }
    bool setIndexRange(Key_t b, Key_t e)
    {
        throw("not implemented");
        return false;
    }
};

#endif
