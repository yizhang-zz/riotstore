#include <iostream>
#include "lower/BitmapPagedFile.h"
#include "BtreeDenseLeafBlock.h"
#include "BtreeSparseBlock.h"
#include "Btree.h"
#include "Config.h"
#include "BtreeStat.h"
#include "BatchBufferFWF.h"
#include "BatchBufferLRU.h"
#include "BatchBufferLS.h"
#include "BatchBufferRand.h"
#include "BatchBufferLG.h"
#include "BatchBufferLGRand.h"

using namespace Btree;

void BTree::init(const char *fileName, int fileFlag)
{
	file = new BitmapPagedFile(fileName, fileFlag);
    buffer = new BufferManager(file, config->btreeBufferSize);
	if (fileFlag&BitmapPagedFile::CREATE)
		buffer->allocatePageWithPID(0, headerPage);
	else
		buffer->readPage(0, headerPage);
    header = (BtreeHeader*) buffer->getPageImage(headerPage);
}

#ifdef USE_BATCH_BUFFER
void BTree::initBatching()
{
	leafHist = new LeafHist(config->batchHistogramNum, header->endsBy);
	switch (config->batchMethod) {
	case kFWF:
		batbuf = new BatchBufferFWF(config->batchBufferSize, this);
		break;
	case kLRU:
		if (config->batchUseHistogram) 
			batbuf = new BatchBufferLRU<HistPageId>(config->batchBufferSize, this);
		else 
			batbuf = new BatchBufferLRU<BoundPageId>(config->batchBufferSize, this);
		break;
	case kLS:
		if (config->batchUseHistogram)
			batbuf = new BatchBufferLS<HistPageId>(config->batchBufferSize, this);
		else
			batbuf = new BatchBufferLS<BoundPageId>(config->batchBufferSize, this);
		break;
	case kLS_RAND:
		if (config->batchUseHistogram)
			batbuf = new BatchBufferRand<HistPageId>(config->batchBufferSize, this);
		else
			batbuf = new BatchBufferRand<BoundPageId>(config->batchBufferSize, this);
		break;
	case kLG:
		if (config->batchUseHistogram)
			batbuf = new BatchBufferLG<HistPageId>(config->batchBufferSize, this);
		else
			batbuf = new BatchBufferLG<BoundPageId>(config->batchBufferSize, this);
		break;
	case kLG_RAND:
		if (config->batchUseHistogram)
			batbuf = new BatchBufferLGRand<HistPageId>(config->batchBufferSize, this);
		else
			batbuf = new BatchBufferLGRand<BoundPageId>(config->batchBufferSize, this);
		break;
	default:
		batbuf = NULL;
	}
}
#endif

BTree::BTree(const char *fileName, Key_t endsBy,
			 LeafSplitter *leafSp, InternalSplitter *intSp
			 ): leafSplitter(leafSp), internalSplitter(intSp)
{
    init(fileName, BitmapPagedFile::CREATE);

    header->endsBy = endsBy;
    header->nLeaves = 0;
    header->depth = 0;
    header->root = INVALID_PID;
    header->firstLeaf = 0;
    buffer->markPageDirty(headerPage);

#ifdef USE_BATCH_BUFFER
	initBatching();
#endif
}

BTree::BTree(const char *fileName, LeafSplitter *leafSp,
			 InternalSplitter *intSp
			 ): leafSplitter(leafSp), internalSplitter(intSp)
{
    init(fileName, 0);
#ifdef USE_BATCH_BUFFER
	initBatching();
#endif

	// TODO : should materialize BTreeStat when the tree is stored on disk
	// and read it back when the tree is loaded
}

BTree::~BTree()
{
#ifdef USE_BATCH_BUFFER
	if (leafHist) {
		//leafHist->print();
		delete leafHist;
	}
	if (batbuf)
		delete batbuf;
#endif
    buffer->unpinPage(headerPage);
    delete buffer;
    delete file;
}

int BTree::search(Key_t key, Cursor *cursor)
{
	// if tree is empty
    if (header->depth == 0) {
        cursor->current = -1;
        return kNotFound;
    }

	int &current = cursor->current;
	for (; current>=0; --current) {
		if (cursor->trace[current]->inRange(key))
			break;
		buffer->unpinPage(cursor->trace[current]->pageHandle);
		delete cursor->trace[current];
	}
	// current should be >=0 if cursor's trace wasn't empty, and -1 if
	// otherwise

	if (current < 0) {
		// start with the root page
		PageHandle ph;
		buffer->readPage(header->root, ph);
		cursor->trace[0] = Block::create(ph, buffer->getPageImage(ph), 0, header->endsBy);
		current = 0;
	}

	Block *block = cursor->trace[current];
    for (int i=current+1; i<header->depth; i++) {
		BlockT<PID_t> *pidblock = static_cast<BlockT<PID_t>*>(block);
        int &idx = cursor->indices[current];
        int ret = block->search(key, idx);
        // idx is the position where the key should be inserted at.
		// To follow the child pointer, the position should be
		// decremented.
        if (ret == kNotFound)
            idx--;
        Key_t l, u;
        PID_t child;
		PageHandle ph;
        pidblock->get(idx, l, child);
        buffer->readPage(child, ph);
        u = pidblock->key(idx+1);
        // load child block
        block = Block::create(ph, buffer->getPageImage(ph), l, u);
        ++current;
        cursor->trace[current] = block;
    }

    // already at the leaf level
    int ret = block->search(key, cursor->indices[current]);
    return ret;
}

#ifdef USE_BATCH_BUFFER
void BTree::locate(Key_t key, HistPageId &pageId)
{
	leafHist->locate(key, pageId);
}

void BTree::locate(Key_t key, BoundPageId &pageId)
//void BTree::locate(Key_t key, PID_t &pid, Key_t &l, Key_t &u)
{
	// If tree is empty, assume every key goes into page 1. Thus at
	// initial stage, all requests in the buffer will have the same
	// destination and will be flushed together. Returning 1 here
	// won't affect future runs.
	Key_t l,u;
    if (header->depth == 0) {
		//pid = 1;
		l = 0;
		u = header->endsBy;
		pageId.lower = l;
		pageId.upper = u;
		return;
    }

	PID_t pid = header->root;
	l=0, u=header->endsBy;
	PageHandle ph;
	InternalBlock *block;

    for (int i=1; i<header->depth; i++) {
		buffer->readPage(pid, ph);
		// read, do not creat!
		block = new InternalBlock(ph, buffer->getPageImage(ph), l, u, false);
        int idx;
        // idx is the position where the key should be inserted at.
		// To follow the child pointer, the position should be
		// decremented.
        if (block->search(key, idx) == kNotFound)
            idx--;
		block->get(idx, l, pid);
        u = block->key(idx+1);
		buffer->unpinPage(ph);
		delete block;
    }
	pageId.lower = l;
	pageId.upper = u;
}
#endif

int BTree::get(const Key_t &key, Datum_t &datum)
{
#ifdef USE_BATCH_BUFFER
	if (batbuf && batbuf->find(key, datum))
		return kOK;
#endif
	
    Cursor cursor(buffer);
    int ret = search(key, &cursor);
    if (ret != kOK) {
        datum = Block::kDefaultValue;
        return kOK;
    }
    LeafBlock *block = static_cast<LeafBlock*>(cursor.trace[cursor.current]);
    return block->get(key, datum);
}

int BTree::put(const Key_t &key, const Datum_t &datum)
{
#ifdef DTRACE_SDT
	RIOT_BTREE_PUT();
#endif
#ifdef USE_BATCH_BUFFER
	if (batbuf) {
		batbuf->put(key, datum);
		return kOK;
	}
#endif
	
    Cursor cursor(buffer);
	cursor.key = key;  // remember this key
    search(key, &cursor);
	return putHelper(key, datum, &cursor);
}

int BTree::putHelper(Key_t key, Datum_t datum, Cursor *cursor)
{
	int ret;
	cursor->key = key;
    if (cursor->current < 0) {
        // tree empty, create it
        PageHandle ph;
        buffer->allocatePage(ph);
        header->depth++;
        header->root = header->firstLeaf = buffer->getPID(ph);
        buffer->markPageDirty(headerPage);

        cursor->current = 0;
		cursor->trace[0] = Block::create(Block::kSparseLeaf,
				ph, buffer->getPageImage(ph), 0, header->endsBy);
        cursor->indices[0] = 0;
		onNewLeaf(cursor->trace[0]);
    }
        
    LeafBlock *block = static_cast<LeafBlock*>(cursor->trace[cursor->current]);
    ret = block->put(key, datum, &cursor->indices[cursor->current]);
    buffer->markPageDirty(block->pageHandle);
    switch (ret) {
	case kOverflow:
        split(cursor);
		break;
#ifndef DISABLE_DENSE_LEAF
	case kSwitchFormat:
		cursor->trace[cursor->current] = block->switchFormat();
		//TODO: if needed, cursor->indices[cursor->current] should be
		//updated by calling the new block's search method
		delete block;
		break;
#endif
	default:
		;
    }
    return ret;
}

void BTree::split(Cursor *cursor)
{
    PageHandle newPh;
    int cur = cursor->current;
    LeafBlock *block = static_cast<LeafBlock*>(cursor->trace[cur]);
	LeafBlock *newLeafBlock;
    buffer->allocatePage(newPh);
    if (leafSplitter->split(&block, &newLeafBlock, newPh, buffer->getPageImage(newPh))) {
		cursor->trace[cur] = block;
	}
	onNewLeaf(newLeafBlock);
    block->setNextLeaf(buffer->getPID(newPh));
    buffer->markPageDirty(block->pageHandle);
    int ret = kOverflow;
	Block *newBlock = newLeafBlock;
	Block *tempBlock = newBlock;
	//PageHandle tempPh = newPh;
	// If the inserted key ends up in the new block
	if (newLeafBlock->inRange(cursor->key)) {
		// the original block won't be in the trace anymore
		// so mark it as temp and it will be deleted shortly
		tempBlock = block;
		// store the new block in the trace
		cursor->trace[cur] = newLeafBlock;
		cursor->indices[cur] -= block->size();
	}
    for (cur--; cur>=0; cur--) {
        PIDBlock *parent = static_cast<InternalBlock*>(cursor->trace[cur]);
        PID_t newPid = buffer->getPID(newBlock->pageHandle);
		int indexTemp;
        ret = parent->put(newBlock->getLowerBound(), newPid, &indexTemp);
		if (newBlock->inRange(cursor->key))
			cursor->indices[cur] = indexTemp;
        buffer->markPageDirty(parent->pageHandle);
        buffer->unpinPage(tempBlock->pageHandle);
        delete tempBlock;
        if (ret != kOverflow)
            break;
        buffer->allocatePage(newPh);
		PIDBlock *newSibling;
        if (internalSplitter->split(&parent, &newSibling, newPh, buffer->getPageImage(newPh))) {
			cursor->trace[cur] = parent;
		}
#ifdef DTRACE_SDT
		RIOT_BTREE_SPLIT_INTERNAL();
#endif
		newBlock = newSibling;
		tempBlock = newSibling;
		if (newSibling->inRange(cursor->key)) {
			tempBlock = parent;
			cursor->trace[cur] = newSibling;
			cursor->indices[cur] -= parent->size();
		}
    }
    if (ret == kOverflow) {
        // overflow has propagated to the root
        PageHandle rootPh;
        buffer->allocatePage(rootPh);
        InternalBlock *newRoot = new InternalBlock(rootPh, buffer->getPageImage(rootPh), 0,
												   header->endsBy, true);
        PID_t oldRootPid = header->root;
		int indexTemp;
        newRoot->put(0, oldRootPid, &indexTemp);
        PID_t newChildPid = buffer->getPID(newPh);
        newRoot->put(newBlock->getLowerBound(), newChildPid, &indexTemp);
		cursor->grow();
		cursor->trace[0] = newRoot;
		if (newBlock->inRange(cursor->key)) {
			cursor->indices[0] = 1;
		}
		else {
			cursor->indices[0] = 0;
		}
        header->root = buffer->getPID(rootPh);
        header->depth++;
        buffer->markPageDirty(headerPage);
        buffer->unpinPage(tempBlock->pageHandle);
        delete tempBlock;
    }
}

void BTree::print(PID_t pid, Key_t beginsAt, Key_t endsBy, int depth)
{
	int indent = 2*depth;
	char buf[indent+1];
	memset(buf, ' ', indent);
	buf[indent] = '\0';
	
	PageHandle ph;
	buffer->readPage(pid, ph);
	Block *block = Block::create(ph, buffer->getPageImage(ph), beginsAt, endsBy);
	std::cout<<buf;
	block->print();
	if (depth < header->depth-1) {
		InternalBlock *iblock = static_cast<InternalBlock*>(block);
		int num = iblock->size();
		int i;
		for (i=0; i<num-1; ++i)
			print(iblock->value(i), iblock->key(i), iblock->key(i+1), depth+1);
		print(iblock->value(i), iblock->key(i), iblock->getUpperBound(), depth+1);
	}
	buffer->unpinPage(ph);
	delete block;
}

void BTree::print()
{
    using namespace std;
    cout<<"BEGIN--------------------------------------"<<endl;
	cout<<"BTree with depth "<<header->depth<<endl;
	if (header->depth > 0)
		print(header->root, 0, header->endsBy, 0);
    cout<<"END----------------------------------------"<<endl;
}


ArrayInternalIterator *BTree::createIterator(IteratorType t, Key_t &beginsAt, Key_t &endsBy)
{
    //if (t == Dense)
	//  return new BTreeDenseIterator(beginsAt, endsBy, this);
    //else
        return NULL;
}

