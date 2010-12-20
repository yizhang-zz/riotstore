#include <gtest/gtest.h>
#include "array/MDArray.h"
#include "array/RowMajor.h"
#include "array/ColMajor.h"
#include "array/BlockBased.h"

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

	// put range [0,1]x[0,1]
	RowMajor<2> rowMajor(&dim[0]);
	MDArray<2> array(dim, DMA, &rowMajor, "array.bin");
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

	// put range [0,1]x[0,1]
	RowMajor<2> rowMajor(&dim[0]);
	MDArray<2> array(dim, BTREE, &rowMajor, "array.bin");
	array.batchPut(begin, end, data);
	checkRect(array, begin, end, data);

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
	
	{
		Matrix array(dim, BTREE, &rowMajor, "array.bin");

		for (int i=0; i<nz; ++i)
			array.put(coords[i], data[i]);
	}
	// array is destructed here
	
	{
		Matrix array("array.bin");
		cholmod_sparse *sparse;
		array.batchGet(MDCoord<2>(0,0), MDCoord<2>(rows-1,cols-1), &sparse);
		array.freeSparse(&sparse);
	}
}

