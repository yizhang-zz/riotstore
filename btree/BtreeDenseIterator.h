#ifndef BTREE_DENSE_ITERATOR_H
#define BTREE_DENSE_ITERATOR_H

#include "../common/ArrayInternalIterator.h"

namespace Btree {
class BTree;

class BTreeDenseIterator : public ArrayInternalIterator
{
private:
    BTree *tree;
    //Block *block;
    //bool atLastBlock;
    Key_t beginsAt;
    Key_t endsBy;
    i64 curKey;
    //PID_t curPid;
    //Key_t plb, pub;

public:
    BTreeDenseIterator(Key_t _beginsAt, Key_t _endsBy, BTree* tree);
    ~BTreeDenseIterator() {}

    bool moveNext();
    //bool movePrev();
    void get(Key_t &k, Datum_t &d);
    void put(const Datum_t &d);
    void reset();
    bool setRange(const Key_t &b, const Key_t &e)
    {
        throw("not implemented");
        return false;
    }
    bool setIndexRange(Key_t b, Key_t e);
};
}

#endif
