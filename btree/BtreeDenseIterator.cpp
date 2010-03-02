#include "BtreeDenseIterator.h"
#include "Btree.h"

using namespace Btree;

BTreeDenseIterator::BTreeDenseIterator(Key_t begin, Key_t end, BTree *tree)
{
    this->tree = tree;
    //this->beginsAt = begin;
    //this->endsBy = end;
    //this->dirty = false;
    //this->block = NULL;
    setIndexRange(begin, end);
}

bool BTreeDenseIterator::setIndexRange(Key_t b, Key_t e)
{
    beginsAt = b;
    endsBy = e;
    reset();
    return true;
}

void BTreeDenseIterator::reset()
{
    curKey = (i64)beginsAt-1;
}

void BTreeDenseIterator::get(Key_t &k, Datum_t &d)
{
    k = curKey;
    tree->get(curKey, d);
}

void BTreeDenseIterator::put(const Datum_t &d)
{
    tree->put(curKey, d);
}

bool BTreeDenseIterator::moveNext()
{
    curKey++;
    return curKey!=endsBy;
}

//bool BTreeDenseIterator::nextBlock()
//{
    /*
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
    */
//}
