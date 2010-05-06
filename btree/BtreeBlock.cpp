#include <iostream>
#include "BtreeBlock.h"
#include "Btree.h"
#include "BtreeConfig.h"

using namespace std;

const int Btree::Block::HeaderSize[4] = {4,8,0,12};
size_t Btree::Block::BlockCapacity[4];
bool Btree::Block::capacityInitialized = initCapacity();

bool Btree::Block::initCapacity()
{
  BlockCapacity[DenseLeaf] = initCapacity(Btree::Block::DenseLeaf);
  BlockCapacity[SparseLeaf] = initCapacity(Btree::Block::SparseLeaf);
  BlockCapacity[Internal] = initCapacity(Btree::Block::Internal);
  return true;
}

size_t Btree::Block::initCapacity(Type t)
{
    size_t unit;
    size_t headerSize;
    const char *name;
    switch(t) {
    case SparseLeaf:
        unit = (sizeof(Key_t)+sizeof(Datum_t));
        headerSize = SparseHeaderSize;
        name = "sleaf_cap";
        break;
    case DenseLeaf:
        unit = sizeof(Datum_t);
        headerSize = DenseHeaderSize;
        name = "dleaf_cap";
        break;
    case Internal:
        unit = sizeof(Key_t)+sizeof(PID_t);
        headerSize = InternalHeaderSize;
        name =  "int_cap";
        break;
    }
    size_t cap = ((PAGE_SIZE)-headerSize)/unit;
    BtreeConfig *config = BtreeConfig::getInstance();
    if (config) {
        const char *val;
        if ((val=config->get(name))) {
            cap = atoi(val);
        }
    }
    return cap;

    //DenseLeafCapacity = (PAGE_SIZE-DenseHeaderSize)/sizeof(Datum_t);
    //SparseLeafCapacity = (PAGE_SIZE-SparseHeaderSize)/(sizeof(Key_t)+sizeof(Datum_t));
    //InternalCapacity = (PAGE_SIZE-InternalHeaderSize)/(sizeof(Key_t)+sizeof(PID_t));
}

Btree::Block::Block(BTree *tree, PageHandle ph, Key_t beginsAt, Key_t endsBy,
                    bool create, Type type)
{
    this->tree = tree;
    this->ph = ph;
    lower = beginsAt;
    upper = endsBy;
    header = tree->getPagePacked(ph);
    u8 *flags = (u8*) header;
    nEntries = (u16*) (flags+2);
    if (create) {
        *nEntries = 0;
        *flags = type;
    }
    else {
    }

	headerSize = HeaderSize[*flags];
	capacity = BlockCapacity[*flags];
    if (*flags & 1) {
        nextLeaf = (PID_t*)(flags + 4);
    }
    else {
        nextLeaf = 0;
    }
    list = (List*) tree->getPageUnpacked(ph);
    // payload = flags + headerSize;
}
        

/*
 * Searches the block for a particular key. If found, index of the entry is
 * written in idx and 0 is returned. Otherwise, idx is set to the index of
 * key if key is to be inserted and 1 is returned.
 */ 
int Btree::Block::search(Key_t key, int &index)
{
	assert(key>=lower && key<upper);
    int size = list->getSize();
	if (size == 0) {
		index = 0;
		return BT_NOT_FOUND;
	}

    Value v;
    int ret = list->search(key, v, index);
    if (ret == List::OK) {
        return BT_OK;
    }
    return BT_NOT_FOUND;
    
	// binary search on the keys
	int p = 0;
	int q = size - 1;
	int mid;
	Key_t *pKeys = (Key_t*) payload;

	do {
		mid = (p+q)/2;
		if (pKeys[mid] > key) q = mid-1;
		else  p = mid+1;
	} while (p <= q && pKeys[mid] != key);

	if (pKeys[mid] == key) {
		index = mid;
		return BT_OK;
	}

	index = p;
	return BT_NOT_FOUND;
}

int Btree::Block::put(Key_t key, const Value &val)
{
    int ret = list->insert(key, val);
    if (ret == List::Overwrite) {
        return BT_OVERWRITE;
    }

    int size = list->getSize();
    if (size == capacity+1) {
        if (isLeaf() && !isDense()) {
            Key_t min, max;
            Value v;
            list->locate(0, min, v);
            list->locate(size-1, max, v);
            if (max-min+1 <= BlockCapacity[DenseLeaf]) {
                switchTo(DenseLeaf);
                return BT_OK;
            }
            return BT_OVERFLOW;
        }
        if (!isLeaf())
            return BT_OVERFLOW;
    }
    if (isLeaf() && isDense()) {
        Key_t min, max;
        Value v;
        list->locate(0, min, v);
        list->locate(size-1, max, v);
        if (max-min+1 > BlockCapacity[DenseLeaf]) {
            if (size <= BlockCapacity[SparseLeaf]) {
                switchTo(SparseLeaf);
                return BT_OK;
            }
            return BT_OVERFLOW;
        }
    }
    return BT_OK;
}

int Btree::Block::get(Key_t key, Value &val)
{
    int index;
    if (list->search(key, val, index) == List::OK)
        return BT_OK;
    return BT_NOT_FOUND;
}

int Btree::Block::del(Key_t key)
{
    if (list->remove(key) == List::OK)
        return BT_OK;
    return BT_NOT_FOUND;
}

Key_t Btree::Block::getKey(int index)
{
    if (index == list->getSize())
        return upper;
    Key_t key;
    Value val;
    get(index, key, val);
    return key;
}

Btree::Value Btree::Block::getValue(int index)
{
    Key_t key;
    Value val;
    get(index, key, val);
    return val;
}

int Btree::Block::get(int index, Key_t &key, Value &val)
{
    assert(index >= 0 && index < list->getSize());
    if (List::OK == list->locate(index, key, val))
        return BT_OK;
    return BT_NOT_FOUND;
}
    
void Btree::Block::truncate(int cutoff)
{
    list->truncate(cutoff);
}

void Btree::Block::switchTo(Type t)
{
    u8 *flags = (u8*)header;
    *flags = t;
    headerSize = HeaderSize[t];
    capacity = BlockCapacity[t];
}

void Btree::Block::print(int depth)
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

void Btree::Block::splitTypes(int sp, Key_t spKey, Type &left, Type &right)
{
  if (!isLeaf()) {
	left = Internal;
	right = Internal;
  }
  else if (!isDense()) {
	left = SparseLeaf;
	right = SparseLeaf;
	if (upper-spKey <= BlockCapacity[DenseLeaf])
	  right = DenseLeaf;
  }
  else {
	left = DenseLeaf;
	right = DenseLeaf;
	if (getSize()-sp <= BlockCapacity[SparseLeaf]
		&& upper-spKey > BlockCapacity[DenseLeaf])
	  right = SparseLeaf;
  }
}

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
