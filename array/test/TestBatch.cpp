#include <gtest/gtest.h>
#include <iostream>
#include "array/MDArray.h"
#include "array/RowMajor.h"
#include "array/ColMajor.h"
#include "array/BlockBased.h"
#include "btree/Btree.h"

using namespace std;

void checkRect(const MDArray<2> &array, const MDCoord<2> &begin, const MDCoord<2> &end, const Datum_t *data)
{
	int k = 0;
	Datum_t datum;
	for (i64 j=begin[1]; j<=end[1]; ++j) 
		for (i64 i=begin[0]; i<=end[0]; ++i) {
			array.get(MDCoord<2>(i,j), datum);
			ASSERT_EQ(data[k++], datum);
		}
}

TEST(MDArray, BatchPutDMA)
{
	MDCoord<2> dim(100,100);

	const int rows = 20;
	const int cols = 20;
	const int size = rows * cols;
	const MDCoord<2> begin(3,5);
	const MDCoord<2> end = begin + MDCoord<2>(rows,cols) - MDCoord<2>(1,1);
	Datum_t data[size];
	for (int i=0; i<size; ++i)
		data[i] = i+1;

    StorageParam sp;
    sp.type = DMA;
    sp.fileName = "array.bin";
	// put range [0,1]x[0,1]
	RowMajor<2> rowMajor(&dim[0]);
	MDArray<2> array(&sp, dim, &rowMajor);
	array.batchPut(begin, end, data);
	checkRect(array, begin, end, data);

	ColMajor<2> colMajor(&dim[0]);
	array.setLinearization(&colMajor);
	array.batchPut(begin, end, data);
	checkRect(array, begin, end, data);

	i64 blockDims[] = {3,2};
	u8 orders[] = {1, 0};
	BlockBased<2> blockBased(&dim[0], blockDims, orders, orders);
	array.setLinearization(&blockBased);
	array.batchPut(begin, end, data);
	checkRect(array, begin, end, data);
}

TEST(MDArray, BatchPutBtree)
{
	MDCoord<2> dim(100,100);

	const int rows = 20;
	const int cols = 20;
	const int size = rows * cols;
	const MDCoord<2> begin(3,5);
	const MDCoord<2> end = begin + MDCoord<2>(rows,cols) - MDCoord<2>(1,1);
	Datum_t data[size];
	for (int i=0; i<size; ++i)
		data[i] = i+1;

    StorageParam sp;
    sp.type = BTREE;
    sp.fileName = "array.bin";
    sp.intSp = 'M';
    sp.leafSp = 'M';
    sp.useDenseLeaf = false;//config->useDenseLeaf;
	// put range [0,1]x[0,1]
	RowMajor<2> rowMajor(&dim[0]);
	MDArray<2> array(&sp, dim, &rowMajor);
	array.batchPut(begin, end, data);
	checkRect(array, begin, end, data);
    ((Btree::BTree*)array.storage)->print(true);

    ASSERT_DOUBLE_EQ(double(size)/(dim[0]*dim[1]), array.sparsity());

	ColMajor<2> colMajor(&dim[0]);
	array.setLinearization(&colMajor);
	array.batchPut(begin, end, data);
	checkRect(array, begin, end, data);

	i64 blockDims[] = {3,2};
	u8 orders[] = {1, 0};
	BlockBased<2> blockBased(&dim[0], blockDims, orders, orders);
	array.setLinearization(&blockBased);
	array.batchPut(begin, end, data);
	checkRect(array, begin, end, data);
}

TEST(MDArray, BatchGetSparse)
{
	const int rows = 20;
	const int cols = 20;
	const int size = rows * cols;
	MDCoord<2> dim(rows,cols);

	const int nz = 5;
	MDCoord<2> coords[nz] = {MDCoord<2>(1,1), MDCoord<2>(3,1), MDCoord<2>(5,3),
		MDCoord<2>(5,9), MDCoord<2>(16,19)};
	Datum_t data[nz] = {1.0, 2.0, 3.0, 4.0, 5.0};

	RowMajor<2> rowMajor(&dim[0]);
	
    StorageParam sp;
    sp.type = BTREE;
    sp.fileName = "array.bin";
    sp.intSp = 'M';
    sp.leafSp = 'B';
    sp.useDenseLeaf = config->useDenseLeaf;

	{
		Matrix array(&sp, dim, &rowMajor);

		for (int i=0; i<nz; ++i)
			array.put(coords[i], data[i]);
	}
	// array is destructed here
	
	{
		Matrix array(sp.fileName);
		SparseMatrix sp = array.batchGet(MDCoord<2>(0,0), MDCoord<2>(rows-1,cols-1));
        SparseMatrix::Iterator it = sp.begin();
        int i = 0;
        for (; it != sp.end(); ++it)
            ASSERT_DOUBLE_EQ(data[i++], it->datum);
	}
}

TEST(MDArray, BatchPutSparse)
{
	const int rows = 100;
	const int cols = 100;
	const int size = rows * cols;
	MDCoord<2> dim(rows,cols);

	const int nz = 100;
    Key_t keys[nz];
    for (int i=0; i<nz; ++i) {
        keys[i] = i;
    }
    SparseMatrix::Element elements[nz];
    permute(keys, nz);
    for (int i=0; i<nz; ++i) 
        elements[i].coord[0] = keys[i];
    permute(keys, nz);
    for (int i=0; i<nz; ++i) {
        elements[i].coord[1] = keys[i];
        elements[i].datum = keys[i]+1;
    }
    
    MDCoord<2> begin(0,0), end(rows-1,cols-1);
    SparseMatrix sm(elements, nz, begin, end, false);

	RowMajor<2> rowMajor(&dim[0]);
    StorageParam sp;
    sp.type = BTREE;
    sp.fileName = "batchputsparse.bin";
    sp.intSp = 'M';
    sp.leafSp = 'B';
    sp.useDenseLeaf = config->useDenseLeaf;

    {
        Matrix a(&sp, dim, &rowMajor);
        a.batchPut(begin, sm);
        for (int i=0; i<nz; ++i) {
            Datum_t x;
            a.get(elements[i].coord, x);
            ASSERT_DOUBLE_EQ(elements[i].datum, x);
        }
    }

    {
        Matrix b(sp.fileName);
        for (int i=0; i<nz; ++i) {
            Datum_t x;
            b.get(elements[i].coord, x);
            ASSERT_DOUBLE_EQ(elements[i].datum, x);
        }
    }
}
