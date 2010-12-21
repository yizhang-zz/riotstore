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
    delete[] initial;
    delete block;
}

TEST(Arith, DenseDenseMult)
{
    i64 arrayDims[] = {20L, 18L};
    i64 blockDims[] = {8L, 7L};
    u8  orders[] = {0,1};
    i64 rows = arrayDims[0];
    i64 cols = arrayDims[1];
	i64 total = rows*cols;
	Datum_t *initial = new Datum_t[total];
	for (int i=0; i<total; ++i)
		initial[i] = i;

    BlockBased<2> *block = new BlockBased<2>(arrayDims, blockDims, orders, orders);
    Linearization<2> *block1 = block->transpose();
	Matrix a(MDCoord<2>(arrayDims), BTREE, block, "a.bin");
	a.batchPut(MDCoord<2>(0,0), MDCoord<2>(rows-1, cols-1), initial);
    Matrix b(MDCoord<2>(cols, rows), BTREE, block1, "b.bin"); // b=a'
    b.batchPut(MDCoord<2>(0,0), MDCoord<2>(cols-1, rows-1), initial);

    Matrix c = a*b;

    // check correctness against in-memory blas dgemm
    Datum_t *result = new Datum_t[rows*rows];
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, rows, rows, cols,
            1.0, initial, rows, initial, cols, 0.0, result, rows);

    for (int i=0; i<rows; ++i)
        for (int j=0; j<cols; ++j) {
            Datum_t d;
            c.get(MDCoord<2>(i,j), d);
            ASSERT_DOUBLE_EQ(result[j*rows+i], d);
        }
    delete[] result;
    delete[] initial;
    delete block;
    delete block1;
}

TEST(Arith, SparseSparseMult)
{
    i64 arrayDims[] = {2200L, 2100L};
    i64 blockDims[] = {1200L, 1100L};
    u8  orders[] = {0,1};
    i64 rows = arrayDims[0];
    i64 cols = arrayDims[1];

	i64 total = rows*cols/100; // sparsity=1/100
    SparseMatrix::Element *elements = new SparseMatrix::Element[total];
    for (int i=0; i<total; ++i) {
        elements[i].coord = Matrix::Coord(rand() % rows, rand() % cols);
        elements[i].datum = i+1;
    }

    BlockBased<2> *block = new BlockBased<2>(arrayDims, blockDims, orders, orders);
	Matrix a(MDCoord<2>(arrayDims), BTREE, block, "a.bin");
    MDCoord<2> begin(0, 0), end(rows-1, cols-1);
	SparseMatrix asp(elements, total, begin, end, false);
    a.batchPut(begin, asp);

    Linearization<2> *block1 = block->transpose();
    Matrix b(MDCoord<2>(cols, rows), BTREE, block1, "b.bin");
    for (int i=0; i<total; ++i) {
        i64 temp = elements[i].coord[0];
        elements[i].coord[0] = elements[i].coord[1];
        elements[i].coord[1] = temp;
    }
    end = end.transpose();
    SparseMatrix bsp(elements, total, begin, end, false);
    b.batchPut(begin, bsp);

    // get row 1003 of a and col 0 of b to calculate c[1003,0]
    /*
    double row[2100];
    double col[2100];
    double res = 0;
    for (int i=0; i<cols; i++) {
        a.get(MDCoord<2>(1003,i), row[i]);
        b.get(MDCoord<2>(i,0), col[i]);
        res += row[i]*col[i];
    }
    cout<<"c[1003,0]="<<res<<endl;
    */

    Matrix c = a*b;

    // check correctness against in-memory cholmod_ssmult
    SparseMatrix csp = asp * bsp;
    Datum_t t;
    c.get(MDCoord<2>(1003,0), t);
    cout<<t<<endl;
    for (SparseMatrix::Iterator it = csp.begin();
            it != csp.end();
            ++it) {
            Datum_t d;
            c.get(it->coord, d);
            ASSERT_DOUBLE_EQ(it->datum, d)<<it->coord;
    }
    delete[] elements;
    delete block;
    delete block1;
}
