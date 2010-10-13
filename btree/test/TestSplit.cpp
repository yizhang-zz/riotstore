#include <gtest/gtest.h>
#include <iostream>
#include "../BtreeDenseLeafBlock.h"
#include "../BtreeSparseBlock.h"
#include "../Splitter.h"
#include "../../lower/BitmapPagedFile.h"
#include "../../lower/BufferManager.h"

using namespace Btree;
using namespace std;

extern int BufferSize;
extern const char *fileName;

TEST(DenseBlock, MSplit)
{
    PagedStorageContainer *file = new BitmapPagedFile(fileName, BitmapPagedFile::F_CREATE);
    BufferManager *buffer = new BufferManager(file, BufferSize);
	PageHandle ph;
	ASSERT_TRUE(buffer->allocatePageWithPID(0,ph)==RC_OK);
	Block *block = new DenseLeafBlock((char*)buffer->getPageImage(ph), 0, 10, true);
	buffer->markPageDirty(ph);

	int num = block->capacity+1;
	//Key_t keys[]={0,4,2,3,5,6,1,7};
	Key_t keys[num];
	Value vals[num];
	for (int i=0; i<num; ++i)
		keys[i] = i;
	permute(keys, num-1); // don't permute the last element
	for (int i=0; i<num; ++i)
		cout<<keys[i]<<" ";
	cout<<endl;
	for (int i=0; i<num; ++i)
		vals[i].datum = keys[i]+.5;
	int ret;
	for (int i=0; i<num; ++i)
		ret = block->put(keys[i], vals[i]);
	ASSERT_EQ(Block::kOverflow, ret);
	cout<<"before split"<<endl;
	block->print();
	Splitter *sp = new MSplitter();
	Block *new_block;
	PageHandle new_ph;
	buffer->allocatePage(new_ph);
	sp->split(&block, &new_block, (char*)buffer->getPageImage(new_ph));
	cout<<"after split"<<endl;
	block->print();
	new_block->print();

	delete new_block;
	delete block;
	delete buffer;
	delete file;
}

TEST(SparseBlock, MSplit)
{
    PagedStorageContainer *file = new BitmapPagedFile(fileName, BitmapPagedFile::F_CREATE);
    BufferManager *buffer = new BufferManager(file, BufferSize);
	PageHandle ph;
	ASSERT_TRUE(buffer->allocatePageWithPID(0,ph)==RC_OK);
	Block *block = new SparseLeafBlock((char*)buffer->getPageImage(ph), 0, 100, true);
	buffer->markPageDirty(ph);

	int num = block->capacity+1;
	Key_t keys[num];
	Value vals[num];
	for (int i=0; i<num; ++i)
		keys[i] = i*2; // prevents it to switch to the dense format
	permute(keys, num); // don't permute the last element
	for (int i=0; i<num; ++i)
		vals[i].datum = keys[i]+.5;
	int ret;
	for (int i=0; i<num; ++i)
		ret = block->put(keys[i], vals[i]);
	ASSERT_EQ(Block::kOverflow, ret);
	cout<<"before split"<<endl;
	block->print();
	Splitter *sp = new MSplitter();
	Block *new_block;
	PageHandle new_ph;
	buffer->allocatePage(new_ph);
	sp->split(&block, &new_block, (char*)buffer->getPageImage(new_ph));
	cout<<"after split"<<endl;
	block->print();
	new_block->print();

	delete new_block;
	delete block;
	delete buffer;
	delete file;
}
