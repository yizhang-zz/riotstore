#include <iostream>
#include "BtreeBlock.h"
#include "BtreeDenseLeafBlock.h"
#include "BtreeSparseBlock.h"
#include "Config.h"

using namespace std;
using namespace Btree;


Block * Block::create(Type t, PageHandle ph, char *image, Key_t beginsAt, Key_t endsBy)
{
	switch (t) {
	case kInternal:
		return new InternalBlock(ph, image, beginsAt, endsBy, true);
	case kDenseLeaf:
		return new DenseLeafBlock(ph, image, beginsAt, endsBy, true);
	case kSparseLeaf:
		return new SparseLeafBlock(ph, image, beginsAt, endsBy, true);
	default:
		return NULL;
	}
}

Block * Block::create(PageHandle ph, char *image, Key_t beginsAt, Key_t endsBy)
{
	// first byte of the block image contains the type
	switch (*image) {
	case kInternal:
		return new InternalBlock(ph, image, beginsAt, endsBy, false);
	case kDenseLeaf:
		return new DenseLeafBlock(ph, image, beginsAt, endsBy, false);
	case kSparseLeaf:
		return new SparseLeafBlock(ph, image, beginsAt, endsBy, false);
	default:
		return NULL;
	}
}

/*
void print(int depth)
{
    using namespace std;
    int size = list->getSize();
    for (int i=0; i<depth; i++)
        cout<<"  ";
    cout<<"|_";
    if (isLeaf()) {
        if (isDense())
            cout<<" D";
        else
            cout<<" S";
        cout<<"["<<lower<<","<<upper<<")\t{";
        List::Iterator *it = list->getIterator();
        Key_t key;
        Value val;
        while(it->moveNext()) {
            it->get(key, val);
            cout<<key<<", ";
        }
        delete it;
        cout<<"}"<<endl;
    }
    else {
        cout<<" I["<<lower<<","<<upper<<")\t{";
        List::Iterator *it = list->getIterator();
        Key_t key;
        Value val;
        while(it->moveNext()) {
            it->get(key, val);
            cout<<key<<", ("<<val.pid<<"), ";
        }
        delete it;
        cout<<"}"<<endl;

        if (tree != NULL && size > 0) {
            PageHandle ph;
            List::Iterator *it = list->getIterator();
            Key_t key1, key2;
            Value val1, val2;
            it->moveNext();
            it->get(key1, val1);
            while(it->moveNext()) {
                it->get(key2, val2);
                ph = tree->loadPage(val1.pid);
                Block *child = new Block(tree, ph, key1, key2);
                child->print(depth+1);
                delete child;
                tree->releasePage(ph);
                key1 = key2;
                val1 = val2;
            }
            delete it;
            // print last child
            ph = tree->loadPage(val1.pid);
            Block *child = new Block(tree, ph, key1, upper);
            child->print(depth+1);
            delete child;
            tree->releasePage(ph);
        }
    }
}
*/

void Block::splitTypes(int sp, Key_t spKey, Type *left, Type *right)
{
  if (!isLeaf()) {
	*left = kInternal;
	*right = kInternal;
  }
  else if (isDense()) {
	*left = kSparseLeaf;
	*right = kSparseLeaf;
	// if left can still fit in a dense format, then keep it
	// that way
	if (spKey-lower <= config->denseLeafCapacity)
		*left = kDenseLeaf;
	if (sizeWithOverflow()-sp > config->sparseLeafCapacity
		|| upper-spKey <= config->denseLeafCapacity)
	  *right = kDenseLeaf;
  }
  else {
	*left = kSparseLeaf;
	*right = kSparseLeaf;
	if (upper-spKey <= config->denseLeafCapacity)
	  *right = kDenseLeaf;
  }
}

/*
Btree::Block *Btree::Block::split(PageHandle newPh, int sp, Key_t spKey)
{
    Block *newBlock;
	Type left, right;
	splitTypes(sp, spKey, left, right);

	newBlock = new Block(tree, newPh, spKey, upper, true, right);

    List::Iterator *it = list->getIterator(sp, list->getSize());
    Key_t key;
    Value val;
    while (it->moveNext()) {
        it->get(key, val);
        newBlock->put(key, val);
    }
    delete it;
    truncate(sp);
    return newBlock;
}

///////////////// sparse iterators //////////////////////

Btree::Block::Iterator* Btree::Block::getSparseIterator(
    int beginsAt, int endsBy)
{
    Iterator* itor = new SparseIterator(this);
    itor->setIndexRange(beginsAt, endsBy);
    return itor;
}

Btree::Block::Iterator* Btree::Block::getSparseIterator()
{
    Iterator* itor = new SparseIterator(this);
    return itor;
}

Btree::Block::SparseIterator::SparseIterator(Block *block)
{
    this->block = block;
    index = -1;
    size = block->getSize();
    begin = 0;
    end = size;
}

bool Btree::Block::SparseIterator::moveNext()
{
    return (++index < end);
}

bool Btree::Block::SparseIterator::movePrev()
{
    return (--index >= begin);
}

void Btree::Block::SparseIterator::get(Key_t &k, Value &d)
{
    block->get(index, k, d);
}

void Btree::Block::SparseIterator::put(const Value &d)
{
    Key_t k;
    Value v;
    int index;
    block->get(index, k, v);
    block->put(k, d);
}

void Btree::Block::SparseIterator::reset()
{
    index = begin - 1;
}

bool Btree::Block::SparseIterator::setIndexRange(Key_t b, Key_t e)
{
    begin = b;
    end = e;
    if (b < 0) begin = 0;
    if (e > size) end = size;
    if (begin >= end)
        return false;
    reset();
    return true;
}

bool Btree::Block::SparseIterator::setRange(const Key_t &b, const Key_t &e)
{
    throw("not implemented");
    return false;
}
/////////////////// dense iterator ////////////////////////


Btree::Block::Iterator* Btree::Block::getDenseIterator(
    const Key_t &beginsAt, const Key_t &endsBy)
{
    Iterator* itor = new DenseIterator(this);
    itor->setRange(beginsAt, endsBy);
    return itor;
}

Btree::Block::Iterator* Btree::Block::getDenseIterator()
{
    Iterator* itor = new DenseIterator(this);
    return itor;
}

Btree::Block::DenseIterator::DenseIterator(Block *block)
{
    this->block = block;
    size = block->getSize();
    setRange(block->getLowerBound(), block->getUpperBound());
}

bool Btree::Block::DenseIterator::moveNext()
{
    if (curKey >= endKey-1)
        return false;

    if (curKey >= lastFetchedKey) {
        ++index;
        if (index < size)
            block->get(index, lastFetchedKey, lastFetchedVal);
    }
    curKey++;
    return true;
}

bool Btree::Block::DenseIterator::movePrev()
{
    if (curKey == beginKey)
        return false;

    if (curKey <= lastFetchedKey) {
        --index;
        if (index >= 0)
            block->get(index, lastFetchedKey, lastFetchedVal);
    }
    curKey--;
    return true;
}

void Btree::Block::DenseIterator::get(Key_t &k, Value &d)
{
    if (curKey == lastFetchedKey) {
        k = lastFetchedKey;
        d = lastFetchedVal;
    }
    else {
        k = curKey;
        d.datum = Block::defaultValue;
    }
}

void Btree::Block::DenseIterator::put(const Value &d)
{
    block->put(curKey, d);
}

void Btree::Block::DenseIterator::reset()
{
    block->search(beginKey, index);
    block->get(index, lastFetchedKey, lastFetchedVal);
    curKey = beginKey - 1;
}

bool Btree::Block::DenseIterator::setRange(const Key_t &b, const Key_t &e)
{
    if (e > block->getUpperBound()) endKey = block->getUpperBound();
    beginKey = b;
    if (beginKey >= endKey) return false;

    reset();
    return true;
}

bool Btree::Block::DenseIterator::setIndexRange(Key_t b, Key_t e)
{
    throw("not implemented");
    return false;
}
*/
