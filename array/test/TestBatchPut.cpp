#include <gtest/gtest.h>
#include "array/MDArray.h"
#include "array/RowMajor.h"
#include "array/ColMajor.h"
#include "array/BlockBased.h"
#include "array/RBParser.h"
//#include "array/Parser.h"


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

TEST(MDArray, BatchPutFromFileToBtree)
{
  const int rows = 1965;
  const int cols = 1035;
  const MDCoord<2> begin(0,0);
  const MDCoord<2> end = begin + MDCoord<2>(rows,cols) - MDCoord<2>(1,1);

  MDCoord<2> dim(rows,cols);
  RowMajor<2> rowMajor(&dim[0]);
  
  // must specify the number of column pointers and 
  // row indexes per line in the original file
  int colPerLine = 13;
  int rowPerLine = 16;

  RBParser<2> parser(colPerLine, rowPerLine);
  const int bufferSize = 5000;
  MDArray<2> array("Maragal_4.rb", &rowMajor, &parser, bufferSize);
  Datum_t datum;
  for (i64 j=begin[1]; j<=end[1]; ++j) {
    for (i64 i=begin[0]; i<=end[0]; ++i) {
      array.get(MDCoord<2>(i,j), datum);
      if (datum != 0) 
	cout << "[Test] " << MDCoord<2>(i,j) << ", " << datum << endl;
    }
  }
}

