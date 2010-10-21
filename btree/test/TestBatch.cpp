#include <gtest/gtest.h>
#include <iostream>
#include "btree/Btree.h"
#include "lower/PagedStorageContainer.h"

using namespace std;
using namespace Btree;

static LeafSplitter *lsp;
static InternalSplitter *isp;

TEST(Batch, FWF)
{
	lsp = new MSplitter<Datum_t>();
	isp = new MSplitter<PID_t>();

	const int rows = 100;
	const int cols = 100;
	BTree *tree = new BTree("batch.bin", rows*cols, lsp, isp);

	PagedStorageContainer::resetPerfCounts();
	
	for (int i=0; i<rows; ++i)
		for (int j=0; j<cols; ++j) {
			tree->put(j*cols+i, 1.0);
		}

	cout<<"reads\t"<<PagedStorageContainer::readCount<<endl
		<<"writes\t"<<PagedStorageContainer::writeCount<<endl;
	
	Datum_t datum;
	for (int i=0; i<rows; ++i)
		for (int j=0; j<cols; ++j) {
			tree->get(i, datum);
			ASSERT_DOUBLE_EQ(1.0, datum);
		}
}
	
