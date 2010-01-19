#include "BtreeBlock.h"
#include "BtreeIntBlock.h"
#include "BtreeSLeafBlock.h"
#include "BtreeDLeafBlock.h"

#include "Btree.h"
#include "../lower/BitmapPagedFile.h"
#include "../lower/LRUPageReplacer.h"
#include <iostream>

Btree::Btree(const char *fileName, u32 endsBy,
        Splitter *leafSp, Splitter *intSp)
{
    file = new BitmapPagedFile(fileName, BitmapPagedFile::F_CREATE);
    buffer = new BufferManager<>(file, BtreeBufferSize);
    assert(buffer->allocatePageWithPID(0, headerPage) == RC_SUCCESS);
    header = (BtreeHeader*) (*headerPage.image);
    header->endsBy = endsBy;
    header->nLeaves = 0;
    header->depth = 0;
    header->root = 0;
    header->firstLeaf = 0;
    buffer->markPageDirty(headerPage);

    leafSplitter = leafSp;
    internalSplitter = intSp;
}

Btree::Btree(const char *fileName,
        Splitter *leafSp, Splitter *intSp)
{
    file = new BitmapPagedFile(fileName, BitmapPagedFile::F_NO_CREATE);
    buffer = new BufferManager<>(file, BtreeBufferSize);
    headerPage.pid = 0;
    assert(buffer->readPage(headerPage) == RC_SUCCESS);
    header = (BtreeHeader*) (*headerPage.image);

    leafSplitter = leafSp;
    internalSplitter = intSp;
}

Btree::~Btree()
{
    delete buffer;
    delete file;
}

int Btree::search(Key_t key, BtreeCursor *cursor)
{
    if (header->depth == 0) {
        cursor->current = -1;
        return BT_NOT_FOUND;
    }

    // start with the root page
    PageHandle ph;
    ph.pid = header->root;
    buffer->readPage(ph);
    BtreeBlock *block = BtreeBlock::load(&ph, 0, header->endsBy);
    cursor->trace[0] = block;
    cursor->current = 0;

    for (int i=1; i<header->depth; i++) {
        int &idx = cursor->indices[cursor->current];
        int ret = block->search(key, idx);
        if (ret == BT_NOT_FOUND)
            idx--;
        Value val = block->getValue(idx);
        PID_t child = val.pid;
        ph.pid = child;
        buffer->readPage(ph);
        Key_t l = block->getKey(idx);
        Key_t u = block->getKey(idx+1);
        // load child block
        block = BtreeBlock::load(&ph, l, u);
        ++(cursor->current);
        cursor->trace[cursor->current] = block;
    }

    // already at the leaf level
    int ret = block->search(key, cursor->indices[cursor->current]);
    return ret;
}

void Btree::loadPage(PageHandle &ph)
{
    buffer->readPage(ph);
}

void Btree::releasePage(const PageHandle &ph)
{
    buffer->unpinPage(ph);
}

int Btree::put(Key_t &key, Datum_t &datum)
{
    BtreeCursor cursor(this);

    int ret = search(key, &cursor);
    if (cursor.current < 0) {
        // tree empty, create it
        PageHandle ph;
        buffer->allocatePage(ph);
        header->depth++;
        header->root = ph.pid;
        header->nLeaves++;
        header->firstLeaf = ph.pid;
        cursor.current = 0;
        cursor.trace[cursor.current] = new BtreeSLeafBlock(&ph, 0, header->endsBy);
        cursor.indices[cursor.current] = 0;
    }
        
    BtreeBlock *block = cursor.trace[cursor.current];
    Entry e;
    e.key = key;
    e.value.datum = datum;
    ret = block->put(cursor.indices[cursor.current], e);
    buffer->markPageDirty(block->getPageHandle());
    if (ret == BT_OVERFLOW) {
        // first try pack (convert to another format)
        BtreeBlock *newBlock = block->pack();
        if (newBlock) {
            delete cursor.trace[cursor.current];
            cursor.trace[cursor.current] = newBlock;
            ret = BT_OK;
        }
        else
            split(&cursor);
    }
    return ret;
}

void Btree::split(BtreeCursor *cursor)
{
    PageHandle newPh;
    int cur = cursor->current;
    BtreeBlock *block = cursor->trace[cur];
    buffer->allocatePage(newPh);
    BtreeBlock *newBlock = leafSplitter->split(block, &newPh);
    block->setNextLeaf(newPh.pid);
    buffer->markPageDirty(block->getPageHandle());
    Entry e;
    int ret = BT_OVERFLOW;
    for (int i=cur-1; i>=0; i--) {
        BtreeBlock *parent = cursor->trace[i];
        u16 pos = cursor->indices[i] + 1;
        e.key = newBlock->getLowerBound();
        e.value.pid = newPh.pid;
        ret = parent->put(pos, e);
        buffer->markPageDirty(parent->getPageHandle());
        if (ret != BT_OVERFLOW)
            break;
        buffer->allocatePage(newPh);
        newBlock = internalSplitter->split(parent, &newPh);
    }
    if (ret == BT_OVERFLOW) {
        // overflow has propagated to the root
        PageHandle rootPh;
        buffer->allocatePage(rootPh);
        BtreeBlock *newRoot = new BtreeIntBlock(&rootPh, 0, header->endsBy);
        e.key = 0;
        e.value.pid = cursor->trace[0]->getPageHandle().pid;
        newRoot->put(0, e);
        e.key = newBlock->getLowerBound();
        e.value.pid = newPh.pid;
        newRoot->put(1, e);
        header->root = rootPh.pid;
        header->depth++;
    }
}

void Btree::print()
{
    using namespace std;
    PageHandle ph;
    ph.pid = header->root;
    buffer->readPage(ph);
    BtreeBlock *root = BtreeBlock::load(&ph, 0, header->endsBy);
    cout<<"-----------------------------------------"<<endl;
    root->print(0, this);
    delete root;
    buffer->unpinPage(ph);
}
