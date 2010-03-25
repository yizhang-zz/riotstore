#include <iostream>

#include "../lower/BitmapPagedFile.h"
#include "BtreeBlock.h"
#include "Btree.h"
#include "BtreeDenseIterator.h"

using namespace Btree;

int BTree::BufferSize = BTree::getBufferSize();
int BTree::getBufferSize() {
    int n = 5000;
    if (getenv("RIOT_BTREE_BUFFER") != NULL) {
        n = atoi(getenv("RIOT_BTREE_BUFFER"));
    }
    debug("Using buffer size %dKB for Btree", n*4);
    return n;
}

Btree::BTree::BTree(const char *fileName, u32 endsBy,
        Splitter *leafSp, Splitter *intSp)
{
    file = new BitmapPagedFile(fileName, BitmapPagedFile::F_CREATE);
    buffer = new BufferManager(file, BufferSize);
    packer = new BtreePagePacker;
    buffer->setPagePacker(packer);
    assert(buffer->allocatePageWithPID(0, headerPage) == RC_OK);
    header = (BTreeHeader*) getPagePacked(headerPage);
    header->endsBy = endsBy;
    header->nLeaves = 0;
    header->depth = 0;
    header->root = 0;
    header->firstLeaf = 0;
    buffer->markPageDirty(headerPage);

    leafSplitter = leafSp;
    internalSplitter = intSp;
}

Btree::BTree::BTree(const char *fileName,
        Splitter *leafSp, Splitter *intSp)
{
    file = new BitmapPagedFile(fileName, BitmapPagedFile::F_NO_CREATE);
    buffer = new BufferManager(file, BufferSize);
    packer = new BtreePagePacker;
    buffer->setPagePacker(packer);

    assert(buffer->readPage(0, headerPage) == RC_OK);
    header = (BTreeHeader*) getPagePacked(headerPage);

    leafSplitter = leafSp;
    internalSplitter = intSp;
}

Btree::BTree::~BTree()
{
    releasePage(headerPage);
    delete buffer;
    delete file;
    delete packer;
}

int Btree::BTree::search(Key_t key, Cursor *cursor)
{
    if (header->depth == 0) {
        cursor->current = -1;
        return BT_NOT_FOUND;
    }

    // start with the root page
    PageHandle ph;
    buffer->readPage(header->root, ph);
    Block *block = new Block(this, ph, 0, header->endsBy);
    cursor->trace[0] = block;
    cursor->current = 0;

    for (int i=1; i<header->depth; i++) {
        int &idx = cursor->indices[cursor->current];
        int ret = block->search(key, idx);
        /* idx is the position where the key should be inserted at.
         * To follow the child pointer, the position should be
         * decremented.
         */
        if (ret == BT_NOT_FOUND)
            idx--;
        Key_t l, u;
        Value val;
        block->get(idx, l, val);
        PID_t child = val.pid;
        buffer->readPage(child, ph);
        u = block->getKey(idx+1);
        // load child block
        block = new Block(this, ph, l, u);
        ++(cursor->current);
        cursor->trace[cursor->current] = block;
    }

    // already at the leaf level
    int ret = block->search(key, cursor->indices[cursor->current]);
    return ret;
}

int Btree::BTree::get(const Key_t &key, Datum_t &datum)
{
    Cursor cursor(this);
    int ret = search(key, &cursor);
    if (ret != BT_OK) {
        datum = Block::defaultValue;
        return BT_OK;
    }
    Block *block = cursor.trace[cursor.current];
    Value v;
    block->get(key, v);
    datum = v.datum;
    return ret;
}

int Btree::BTree::put(const Key_t &key, const Datum_t &datum)
{
    Cursor cursor(this);

    int ret = search(key, &cursor);
    if (cursor.current < 0) {
        // tree empty, create it
        PageHandle ph;
        buffer->allocatePage(ph);
        header->depth++;
        header->root = buffer->getPID(ph);
        header->nLeaves++;
        header->firstLeaf = header->root;
        buffer->markPageDirty(headerPage);

        cursor.current = 0;
        cursor.trace[cursor.current] = new Block(this, ph, 0, header->endsBy,
                                                 true, Block::SparseLeaf);
        cursor.indices[cursor.current] = 0;
    }
        
    Block *block = cursor.trace[cursor.current];
    Value v;
    v.datum = datum;
    ret = block->put(key, v);
    buffer->markPageDirty(block->getPageHandle());
    if (ret == BT_OVERFLOW) {
        split(&cursor);
    }
    return ret;
}

void Btree::BTree::split(Cursor *cursor)
{
    PageHandle newPh;
    int cur = cursor->current;
    Block *block = cursor->trace[cur];
    buffer->allocatePage(newPh);
    Block *newBlock = leafSplitter->split(block, newPh);
    //newPh = newBlock->getPageHandle();
    header->nLeaves++;
    block->setNextLeaf(buffer->getPID(newPh));
    buffer->markPageDirty(block->getPageHandle());
    Value v;
    int ret = BT_OVERFLOW;
    for (int i=cur-1; i>=0; i--) {
        Block *parent = cursor->trace[i];
        u16 pos = cursor->indices[i] + 1;
        v.pid = buffer->getPID(newPh);
        ret = parent->put(newBlock->getLowerBound(), v);
        buffer->markPageDirty(parent->getPageHandle());
        buffer->unpinPage(newPh);
        delete newBlock;
        if (ret != BT_OVERFLOW)
            break;
        buffer->allocatePage(newPh);
        newBlock = internalSplitter->split(parent, newPh);
        //newPh = newBlock->getPageHandle();
    }
    if (ret == BT_OVERFLOW) {
        // overflow has propagated to the root
        PageHandle rootPh;
        buffer->allocatePage(rootPh);
        Block *newRoot = new Block(this, rootPh, 0,
                                   header->endsBy, true, Block::Internal);
        v.pid = buffer->getPID(cursor->trace[0]->getPageHandle());
        newRoot->put(0, v);
        v.pid = buffer->getPID(newPh);
        newRoot->put(newBlock->getLowerBound(), v);
        header->root = buffer->getPID(rootPh);
        header->depth++;
        buffer->markPageDirty(headerPage);
        buffer->unpinPage(newPh);
        buffer->unpinPage(rootPh);
        delete newBlock;
        delete newRoot;
    }
}

void Btree::BTree::print()
{
    using namespace std;
    PageHandle ph;
    buffer->readPage(header->root, ph);
    Block *root = new Block(this, ph, 0, header->endsBy);
    cout<<"-----------------------------------------"<<endl;
    root->print(0);
    delete root;
    buffer->unpinPage(ph);
}

ArrayInternalIterator *BTree::createIterator(IteratorType t, Key_t &beginsAt, Key_t &endsBy)
{
    if (t == Dense)
        return new BTreeDenseIterator(beginsAt, endsBy, this);
    else
        return NULL;
}
