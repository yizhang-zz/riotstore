#include <gtest/gtest.h>
#include <iostream>
#include "btree/Btree.h"

using namespace std;
using namespace Btree;

static LeafSplitter *lsp;
static InternalSplitter *isp;

TEST(Btree, Batch)
{
	lsp = new MSplitter<Datum_t>();
	isp = new MSplitter<PID_t>();

	const int rows = 200;
	const int cols = 200;
	BTree *tree = new BTree("batch.bin", rows*cols, lsp, isp);

	for (int i=0; i<rows; ++i)
		for (int j=0; j<cols; ++j) {
			tree->put(j*cols+i, j*cols+i+1.0);
		}

	Datum_t datum;
	for (int i=0; i<rows; ++i)
		for (int j=0; j<cols; ++j) {
			tree->get(i*rows+j, datum);
			ASSERT_DOUBLE_EQ(i*rows+j+1.0, datum)<<" for key="<<i<<endl;
		}
	delete tree;

	// random insertions
	
	tree = new BTree("batch.bin", rows*cols, lsp, isp);
	Key_t keys[rows*cols];
	for (int i=0; i<rows*cols; ++i)
		keys[i] = i;
	permute(keys, rows*cols);
	for (int i=0; i<rows*cols; ++i) {
		tree->put(keys[i], keys[i]+1.0);
	}
	for (int i=0; i<rows*cols; ++i) {
		tree->get(keys[i], datum);
		ASSERT_DOUBLE_EQ(keys[i]+1.0, datum)<<" for key="<<keys[i]<<endl;
	}
	delete tree;
	delete lsp;
	delete isp;
}
