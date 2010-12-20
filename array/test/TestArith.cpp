#include <gtest/gtest.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <gsl/gsl_cblas.h>
#include <iostream>
#include <map>

#include "../MDCoord.h"
#include "../RowMajor.h"
#include "../MDArray.h"

using namespace std;

template<int nDim>
void printMDArray(const MDArray<nDim> &array)
{
	MDCoord<nDim> dim = array.getDims();
	Datum_t datum;
    for (int i=0; i<dim[0]; i++) {
        for (int j=0; j<dim[1]; j++) {
            MDCoord<2> c(i,j);
            array.get(c, datum);
			cout<<datum<<"\t";
           }
        cout<<endl;
    }
}

TEST(Arith, Add)
{
	i64 arrayDims[] = {20L, 18L};
	i64 blockDims[] = {8L, 7L};
	u8  orders[] = {0,1};
	i64 rows = arrayDims[0];
	i64 cols = arrayDims[1];
	BlockBased<2> *block = new BlockBased<2>(arrayDims, blockDims, orders, orders);
	//Matrix a(MDCoord<2>(arrayDims), DMA, block, "a.bin");

	i64 total = rows*cols;
	Datum_t *initial = new Datum_t[total];
	for (int i=0; i<total; ++i)
		initial[i] = i;

	Matrix a(MDCoord<2>(arrayDims), BTREE, block, "a.bin");
	a.batchPut(MDCoord<2>(0,0), MDCoord<2>(rows-1, cols-1), initial);
	//cout<<"a="<<endl;
	//printMDArray(a);

	Matrix b(MDCoord<2>(arrayDims), BTREE, block, "b.bin");
	b.batchPut(MDCoord<2>(0,0), MDCoord<2>(rows-1, cols-1), initial);
	//cout<<"b="<<endl;
	//printMDArray(b);

	a += b;
	//cout<<"a+b="<<endl;
	//printMDArray(a);
	
	// check correctness against in-memory add
	for (int i=0; i<rows; ++i)
		for (int j=0; j<cols; ++j) {
			Datum_t d;
			b.get(MDCoord<2>(i,j), d);
			ASSERT_DOUBLE_EQ(j*rows+i, d)<<"coord=("<<i<<","<<j<<")";
			a.get(MDCoord<2>(i,j), d);
			ASSERT_DOUBLE_EQ(initial[j*rows+i]*2, d);
		}

	Linearization<2> *block1 = block->transpose();
	Matrix t(MDCoord<2>(cols, rows), BTREE, block1, "t.bin");
	//Matrix t(MDCoord<2>(cols, rows), DMA, block1, "t.bin");
	t.batchPut(MDCoord<2>(0,0), MDCoord<2>(cols-1, rows-1), initial);
	//cout<<"t="<<endl;
	//printMDArray(t);

	//cout<<"b*t="<<endl;
	Matrix c = b*t;
	//printMDArray(c);

	// check correctness against blas dgemm
	Datum_t *result = new Datum_t[rows*rows];
	cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, rows, rows, cols,
			1.0, initial, rows, initial, cols, 0.0, result, rows);

	for (int i=0; i<rows; ++i)
		for (int j=0; j<cols; ++j) {
			Datum_t d;
			c.get(MDCoord<2>(i,j), d);
			ASSERT_DOUBLE_EQ(result[j*rows+i], d);
		}
	delete[] initial;
	delete[] result;
	delete block;
	delete block1;
}
