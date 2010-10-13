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
    PagedStorageContainer *file = new BitmapPagedFile(fileName, BitmapPagedFile::F_CREATE);
    BufferManager *buffer = new BufferManager(file, BufferSize);
	PageHandle ph;
	ASSERT_TRUE(buffer->allocatePageWithPID(0,ph)==RC_OK);
	DenseLeafBlock *block = new DenseLeafBlock((char*)buffer->getPageImage(ph), 0, 10, true);
	buffer->markPageDirty(ph);
	delete block;
	delete buffer;
	delete file;

    file = new BitmapPagedFile(fileName, BitmapPagedFile::F_NO_CREATE);
    buffer = new BufferManager(file, BufferSize);
	ASSERT_TRUE(buffer->readPage(0,ph)==RC_OK);
	block = new DenseLeafBlock((char*)buffer->getPageImage(ph), 0, 10, false);
	ASSERT_EQ(block->size(), 0);

	Key_t key = 5;
	Value val;
	val.datum = 1.0;
	block->put(key, val);

	ASSERT_EQ(block->size(), 1);
	ASSERT_EQ(block->get(key, val), Block::kOK);
	ASSERT_DOUBLE_EQ(val.datum, 1.0);

	key = 9;
	block->put(key, val);
	ASSERT_EQ(block->size(), 2);
	ASSERT_EQ(block->get(key, val), Block::kOK);
	ASSERT_DOUBLE_EQ(val.datum, 1.0);
	ASSERT_EQ(block->get(key-1, val), Block::kOK);
	ASSERT_DOUBLE_EQ(val.datum, 0.0);
	ASSERT_EQ(block->get(key-2, val), Block::kOK);
	ASSERT_DOUBLE_EQ(val.datum, 0.0);

	val.datum = 1.0;
	block->put(6, val);
	block->put(7, val);
	block->put(8, val);
	block->put(3, val);

	// 3 should be the first element and 9 the last
	block->get(0, key, val);
	ASSERT_EQ(3, key);
	block->get(5-3, key, val);
	ASSERT_EQ(5, key);
	block->get(9-3, key, val);
	ASSERT_EQ(9, key);

	int ret = block->put(2, val); // causing overflow
	ASSERT_EQ(Block::kOverflow, ret);
	ASSERT_EQ(6, block->size());
	ASSERT_EQ(7, block->sizeWithOverflow());

	block->print();
	delete block;
	delete buffer;
	delete file;
	remove(fileName);
}

TEST(SparseLeafBlock, Create)
{
	Key_t key;
	Value val;
    PagedStorageContainer *file = new BitmapPagedFile(fileName, BitmapPagedFile::F_CREATE);
    BufferManager *buffer = new BufferManager(file, BufferSize);
	PageHandle ph;
	ASSERT_TRUE(buffer->allocatePageWithPID(0,ph)==RC_OK);
	SparseLeafBlock *block = new SparseLeafBlock((char*)buffer->getPageImage(ph), 0, 10, true);

	val.datum = 0.1;
	block->put(0, val);
	val.datum = 5.0;
	block->put(5, val);
	ASSERT_EQ(block->size(), 2);
	
	buffer->markPageDirty(ph);
	delete buffer;
	delete file;

	file = new BitmapPagedFile(fileName, BitmapPagedFile::F_NO_CREATE);
    buffer = new BufferManager(file, BufferSize);
	ASSERT_EQ(buffer->readPage(0,ph), RC_OK);
	block = new SparseLeafBlock((char*)buffer->getPageImage(ph), 0, 10, false);

	ASSERT_EQ(block->size(), 2);
	ASSERT_EQ(block->sizeWithOverflow(), 2);

	delete block;

	buffer->allocatePage(ph);
	block = new SparseLeafBlock((char*)buffer->getPageImage(ph), 0, 10, true);
	// random insertions
	int num_keys = config->sparseLeafCapacity;
	Key_t keys[num_keys];
	for (int i=0; i<num_keys; i++)
		keys[i] = i;
	permute(keys, num_keys);
	val.datum = 1.4;
	for (int i=0; i<num_keys; i++) {
		block->put(keys[i], val);
	}
	for (int i=0; i<num_keys; i++) {
		block->get(i, key, val);
		ASSERT_EQ(i, key);
		ASSERT_DOUBLE_EQ(1.4, val.datum);
	}
	int index;
	for (int i=0; i<num_keys; i++) {
		key = i;
		block->get(key, val);
		ASSERT_DOUBLE_EQ(val.datum, 1.4);
	}
	block->print();
	delete block;
	delete buffer;
	delete file;
	remove(fileName);
}

TEST(InternalBlock, Create)
{
	Key_t key;
	Value val;
    PagedStorageContainer *file = new BitmapPagedFile(fileName, BitmapPagedFile::F_CREATE);
    BufferManager *buffer = new BufferManager(file, BufferSize);
	PageHandle ph;
	ASSERT_TRUE(buffer->allocatePageWithPID(0,ph)==RC_OK);
	InternalBlock *block = new InternalBlock((char*)buffer->getPageImage(ph), 0, 10, true);
	
	ASSERT_EQ(block->size(), 0);
	
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
		val.pid = keys[i];
		block->put(keys[i], val);
	}

	// retrieve and test if in order
	for (int i=0; i<num_keys; i++) {
		block->get(i, key, val);
		ASSERT_EQ(key, i*2);
		ASSERT_EQ(val.pid, i*2);
	}

	// search for non-existent keys
	int index;
	for (int i=0; i<num_keys; i++) {
		key = 2*i+1;
		block->search(key, index);
		ASSERT_EQ(index, i+1);
	}

	block->print();
	delete block;
	delete buffer;
	delete file;
	remove(fileName);
}


TEST(DenseBlock, Switch)
{
    PagedStorageContainer *file = new BitmapPagedFile(fileName, BitmapPagedFile::F_CREATE);
    BufferManager *buffer = new BufferManager(file, BufferSize);
	PageHandle ph;
	ASSERT_TRUE(buffer->allocatePage(ph)==RC_OK);
	DenseLeafBlock *block = new DenseLeafBlock((char*)buffer->getPageImage(ph), 0, 100, true);
	buffer->markPageDirty(ph);

	Value val;
	val.datum=1.0;
	for (int i=0; i<block->capacity/2; ++i)
		block->put(i*2, val);
	int ret;
	ret = block->put(block->capacity, val);
	ASSERT_EQ(Block::kSwitchFormat, ret);
	Block *new_block = block->switchFormat(Block::kSparseLeaf);
	ASSERT_EQ(Block::kSparseLeaf, new_block->type());
	delete block;
	new_block->print();

	delete new_block;
	delete buffer;
	delete file;
}

TEST(SparseBlock, Switch)
{
    PagedStorageContainer *file = new BitmapPagedFile(fileName, BitmapPagedFile::F_CREATE);
    BufferManager *buffer = new BufferManager(file, BufferSize);
	PageHandle ph;
	ASSERT_TRUE(buffer->allocatePage(ph)==RC_OK);
	Block *block = new SparseLeafBlock((char*)buffer->getPageImage(ph), 0, 100, true);
	buffer->markPageDirty(ph);

	Value val;
	val.datum=1.0;
	for (int i=0; i<block->capacity; ++i)
		block->put(i, val);
	int ret;
	ret = block->put(block->capacity, val);
	ASSERT_EQ(Block::kSwitchFormat, ret);
	Block *new_block = block->switchFormat(Block::kDenseLeaf);
	ASSERT_EQ(Block::kDenseLeaf, new_block->type());
	delete block;
	new_block->print();

	delete new_block;
	delete buffer;
	delete file;
}
