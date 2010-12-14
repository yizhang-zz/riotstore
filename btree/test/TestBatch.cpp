#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include "btree/Btree.h"

using namespace std;
using namespace Btree;

static LeafSplitter *lsp;
static InternalSplitter *isp;
/*
TEST(Btree, Batch)
{
	lsp = new MSplitter<Datum_t>();
	isp = new MSplitter<PID_t>();

	const int rows = 400;
	const int cols = 400;
	BTree *tree = new BTree("batch.bin", rows*cols, lsp, isp);
	Datum_t datum;

	tree->put(0, 0.0);
	tree->put(1, 1.0);
	tree->print();
	tree->put(1, 0.0);
	tree->print();

	for (int i=0; i<rows; ++i)
		for (int j=0; j<cols; ++j) {
			tree->put(j*cols+i, (double)j*cols+i);
		}

<<<<<<< Updated upstream
=======
	
	//TIMESTAMP(t2);
	//cout<<"reads\t"<<PagedStorageContainer::readCount<<endl
	//	<<"writes\t"<<PagedStorageContainer::writeCount<<endl;
	//cout<<"I/O time\t"<<PagedStorageContainer::accessTime<<endl
	//	<<"CPU time\t"<<t2-t1<<endl;
	
	Datum_t datum;
>>>>>>> Stashed changes
	for (int i=0; i<rows; ++i)
		for (int j=0; j<cols; ++j) {
			tree->get(i*rows+j, datum);
			ASSERT_DOUBLE_EQ(i*rows+j, datum)<<"for key="<<i*rows+j<<endl;
		}
<<<<<<< Updated upstream
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
=======
}*/

TEST(Btree, BatchPut){
  ofstream outFile("BatchPutTest.out");
  lsp = new MSplitter<Datum_t>();
  isp = new MSplitter<PID_t>();
  int max = 100000000;
  BTree *tree = new BTree("batchput.bin", max, lsp, isp);
  int ret = tree->batchPut("../test.in.50k", 5000);
  Datum_t datum;
  int getret = -1;
  for (int i=0; i<max; i++){
    getret = tree->get(i,datum);
    if (datum != 0){
      outFile << i << " " << datum << endl;
    }
  }  
  outFile.close();
>>>>>>> Stashed changes
}
