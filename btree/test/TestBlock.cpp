#include <gtest/gtest.h>
#include <iostream>
#include "../BtreeDenseLeafBlock.h"
#include "../BtreeSparseBlock.h"
#include "../../lower/BitmapPagedFile.h"
#include "../../lower/BufferManager.h"

using namespace Btree;
using namespace std;

int BufferSize = 10;
const char *fileName = "/riot/a.bt";

// Make sure denseLeafCapacity=7
// sparseLeafCapacity=5
TEST(DenseBlock, Create)
{
    PagedStorageContainer *file = new BitmapPagedFile(fileName, BitmapPagedFile::CREATE);
    BufferManager *buffer = new BufferManager(file, BufferSize);
	PageHandle ph;
	ASSERT_TRUE(buffer->allocatePageWithPID(0,ph)==RC_OK);
	DenseLeafBlock *block = new DenseLeafBlock(ph, buffer->getPageImage(ph), 0, config->denseLeafCapacity+1, true);
	buffer->markPageDirty(ph);
	delete block;
	delete buffer;
	delete file;

    file = new BitmapPagedFile(fileName, 0);
    buffer = new BufferManager(file, BufferSize);
	ASSERT_TRUE(buffer->readPage(0,ph)==RC_OK);
	block = new DenseLeafBlock(ph, buffer->getPageImage(ph), 0, config->denseLeafCapacity+1, false);
	ASSERT_EQ(block->size(), 0);

	Datum_t val = 1.0;
	int index;

	for (int i=1; i<=config->sparseLeafCapacity; ++i)
		block->put(i, val, &index);

	ASSERT_EQ(config->sparseLeafCapacity, block->size());
	ASSERT_EQ(block->get(1, val), kOK);
	ASSERT_DOUBLE_EQ(val, 1.0);

	Key_t key = config->denseLeafCapacity-1;
	block->put(key, val, &index);
	ASSERT_EQ(block->get(key, val), kOK);
	ASSERT_DOUBLE_EQ(val, 1.0);

	val = 1.0;
	block->put(0, val, &index);
	ASSERT_EQ(0, index);
	block->get(0, key, val);
	ASSERT_EQ(0, key);

	int ret = block->put(config->denseLeafCapacity, val, &index); // causing overflow
	ASSERT_EQ(kOverflow, ret);
	ASSERT_EQ(1+block->size(), block->sizeWithOverflow());

	delete block;
	delete buffer;
	delete file;
	remove(fileName);
}

TEST(SparseLeafBlock, Create)
{
	Key_t key;
	Datum_t val;
    PagedStorageContainer *file = new BitmapPagedFile(fileName, BitmapPagedFile::CREATE);
    BufferManager *buffer = new BufferManager(file, BufferSize);
	PageHandle ph;
	ASSERT_TRUE(buffer->allocatePageWithPID(0,ph)==RC_OK);
	SparseLeafBlock *block = new SparseLeafBlock(ph, buffer->getPageImage(ph), 0, config->sparseLeafCapacity, true);

	int index;
	val = 0.1;
	block->put(0, val, &index);
	val = 5.0;
	block->put(5, val, &index);
	ASSERT_EQ(block->size(), 2);
	
	buffer->markPageDirty(ph);
	delete buffer;
	delete file;

	file = new BitmapPagedFile(fileName, 0);
    buffer = new BufferManager(file, BufferSize);
	ASSERT_EQ(buffer->readPage(0,ph), RC_OK);
	block = new SparseLeafBlock(ph, buffer->getPageImage(ph), 0, config->sparseLeafCapacity, false);

	ASSERT_EQ(block->size(), 2);
	ASSERT_EQ(block->sizeWithOverflow(), 2);

	delete block;

	buffer->allocatePage(ph);
	block = new SparseLeafBlock(ph, buffer->getPageImage(ph), 0, config->sparseLeafCapacity, true);
	// random insertions
	int num_keys = config->sparseLeafCapacity;
	Key_t keys[num_keys];
	for (int i=0; i<num_keys; i++)
		keys[i] = i;
	permute(keys, num_keys);
	val = 1.4;
	for (int i=0; i<num_keys; i++) {
		block->put(keys[i], val, &index);
	}
	for (int i=0; i<num_keys; i++) {
		block->get(i, key, val);
		ASSERT_EQ(i, key);
		ASSERT_DOUBLE_EQ(1.4, val);
	}
	for (int i=0; i<num_keys; i++) {
		key = i;
		block->get(key, val);
		ASSERT_DOUBLE_EQ(val, 1.4);
	}
	delete block;
	delete buffer;
	delete file;
	remove(fileName);
}

TEST(InternalBlock, Create)
{
	Key_t key;
	PID_t val;
    PagedStorageContainer *file = new BitmapPagedFile(fileName, BitmapPagedFile::CREATE);
    BufferManager *buffer = new BufferManager(file, BufferSize);
	PageHandle ph;
	ASSERT_TRUE(buffer->allocatePageWithPID(0,ph)==RC_OK);
	InternalBlock *block = new InternalBlock(ph, buffer->getPageImage(ph), 0, config->internalCapacity*10, true);
	
	ASSERT_EQ(block->size(), 0);
	int index;
	
	// random insertions
	int num_keys = config->internalCapacity;
	Key_t keys[num_keys];
	PID_t pids[num_keys];
	for (int i=0; i<num_keys; i++) {
		keys[i] = i*2;
	}

	// randomly permute the keys and insert
	permute(keys, num_keys);
	for (int i=0; i<num_keys; i++) {
		val = keys[i];
		block->put(keys[i], val, &index);
	}

	// retrieve and test if in order
	for (int i=0; i<num_keys; i++) {
		block->get(i, key, val);
		ASSERT_EQ(key, i*2);
		ASSERT_EQ(val, i*2);
	}

	// search for non-existent keys
	for (int i=0; i<num_keys; i++) {
		key = 2*i+1;
		block->search(key, index);
		ASSERT_EQ(index, i+1);
	}

	delete block;
	delete buffer;
	delete file;
	remove(fileName);
}


TEST(DenseBlock, Switch)
{
    config->disableDenseLeaf = 0;
    PagedStorageContainer *file = new BitmapPagedFile(fileName, BitmapPagedFile::CREATE);
    BufferManager *buffer = new BufferManager(file, BufferSize);
	PageHandle ph;
	ASSERT_TRUE(buffer->allocatePage(ph)==RC_OK);
	DenseLeafBlock *block = new DenseLeafBlock(ph, buffer->getPageImage(ph), 0, config->denseLeafCapacity, true);
	buffer->markPageDirty(ph);

	Datum_t val;
	int index;
	val=1.0;
	for (int i=0; i<block->capacity/2; ++i)
		block->put(i*2, val, &index);
	int ret;
	ret = block->put(block->capacity, val, &index);
	ASSERT_EQ(kSwitchFormat, ret);
	Block *new_block = block->switchFormat();
	ASSERT_EQ(Block::kSparseLeaf, new_block->type());
	delete block;

	delete new_block;
	delete buffer;
	delete file;
}

TEST(SparseBlock, Switch)
{
    config->disableDenseLeaf = 0;
    PagedStorageContainer *file = new BitmapPagedFile(fileName, BitmapPagedFile::CREATE);
    BufferManager *buffer = new BufferManager(file, BufferSize);
	PageHandle ph;
	ASSERT_TRUE(buffer->allocatePage(ph)==RC_OK);
	LeafBlock *block = new SparseLeafBlock(ph,buffer->getPageImage(ph), 0, config->denseLeafCapacity, true);
	buffer->markPageDirty(ph);

	Datum_t val;
	val=1.0;
	int index;
	for (int i=0; i<block->capacity; ++i)
		block->put(i, val, &index);
	int ret;
	ret = block->put(block->capacity, val, &index);
	ASSERT_EQ(kSwitchFormat, ret);
	Block *new_block = block->switchFormat();
	ASSERT_EQ(Block::kDenseLeaf, new_block->type());
	delete block;

	delete new_block;
	delete buffer;
	delete file;
}
