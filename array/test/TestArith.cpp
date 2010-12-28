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

    StorageParam sp;
    sp.type = BTREE;
    sp.fileName = "a.bin";
    sp.intSp = 'M';
    sp.leafSp = 'B';
    sp.useDenseLeaf = config->useDenseLeaf;
	Matrix a(&sp, MDCoord<2>(arrayDims), block);
	a.batchPut(MDCoord<2>(0,0), MDCoord<2>(rows-1, cols-1), initial);
	//cout<<"a="<<endl;
	//printMDArray(a);

    sp.fileName = "b.bin";
	Matrix b(&sp, MDCoord<2>(arrayDims), block);
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
    StorageParam sp;
    sp.type = BTREE;
    sp.fileName = "a.bin";
    sp.intSp = 'M';
    sp.leafSp = 'B';
    sp.useDenseLeaf = config->useDenseLeaf;
	Matrix a(&sp, MDCoord<2>(arrayDims), block);
	a.batchPut(MDCoord<2>(0,0), MDCoord<2>(rows-1, cols-1), initial);
    sp.fileName = "b.bin";
    Matrix b(&sp, MDCoord<2>(cols, rows), block1); // b=a'
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
    //i64 arrayDims[] = {8L, 7L};
    //i64 blockDims[] = {3L, 2L};
    u8  orders[] = {0,1};
    i64 rows = arrayDims[0];
    i64 cols = arrayDims[1];

    i64 total = rows*cols/100; // sparsity=1/100
    SparseMatrix::Element *elements = new SparseMatrix::Element[total];
    for (int i=0; i<total; ++i) {
        elements[i].coord = Matrix::Coord(rand() % rows, rand() % cols);
        elements[i].datum = i+1;
	//cout<<elements[i].coord<<'\t'<<elements[i].datum<<endl;
    }

    double *temp = new double[rows*cols];

    BlockBased<2> *block = new BlockBased<2>(arrayDims, blockDims, orders, orders);
    StorageParam sp;
    sp.type = BTREE;
    sp.fileName = "a.bin";
    sp.intSp = 'M';
    sp.leafSp = 'B';
    sp.useDenseLeaf = config->useDenseLeaf;
    Matrix a(&sp, MDCoord<2>(arrayDims), block);
    MDCoord<2> begin(0, 0);
    SparseMatrix asp(elements, total, begin, MDCoord<2>(rows-1, cols-1), false);
    a.batchPut(begin, asp);

    a.batchGet(begin, MDCoord<2>(rows-1, cols-1), temp);
    for (SparseMatrix::Iterator it = asp.begin();
            it != asp.end();
            ++it) {
        ASSERT_DOUBLE_EQ(it->datum, temp[it->coord[0]+it->coord[1]*rows]);
    }

    Linearization<2> *block1 = block->transpose();
    sp.fileName = "b.bin";
    Matrix b(&sp, MDCoord<2>(cols, rows), block1);
    for (int i=0; i<total; ++i) {
        i64 temp = elements[i].coord[0];
        elements[i].coord[0] = elements[i].coord[1];
        elements[i].coord[1] = temp;
    }
    SparseMatrix bsp(elements, total, begin, MDCoord<2>(cols-1, rows-1), false);
    b.batchPut(begin, bsp);

    b.batchGet(begin, MDCoord<2>(cols-1, rows-1), temp);
    for (SparseMatrix::Iterator it = bsp.begin();
            it != bsp.end();
            ++it) {
        ASSERT_DOUBLE_EQ(it->datum, temp[it->coord[0]+it->coord[1]*cols])
            <<it->coord;
    }
    delete[] temp;
    
    Matrix c = a*b;

    // check correctness against in-memory cholmod_ssmult
    SparseMatrix csp = asp * bsp;
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

TEST(Arith, SparseDenseMult)
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
    StorageParam sp;
    sp.type = BTREE;
    sp.fileName = "a.bin";
    sp.intSp = 'M';
    sp.leafSp = 'B';
    sp.useDenseLeaf = config->useDenseLeaf;
    Matrix a(&sp, MDCoord<2>(arrayDims), block);
    MDCoord<2> begin(0, 0), end(rows-1, cols-1);
    SparseMatrix asp(elements, total, begin, end, false);
    a.batchPut(begin, asp);

    // // check a block in a
    // for (int ii=0; ii<3;++ii)
    //     for (int jj=0; jj<3;++jj)
    // 	    {
    // 		SparseMatrix sm = a.batchGet(MDCoord<2>(ii*1000,jj*1000),
    // 					     MDCoord<2>((ii+1)*1000-1,(jj+1)*1000-1));
    // 		cholmod_sparse *sp = sm.storage();
    // 		int *p = (int*) sp->p;
    // 		int *r = (int*) sp->i;
    // 		int *p1 = (int*) asp.storage()->p;
    // 		int *r1 = (int*) asp.storage()->i;
    // 		double *x1 = (double*) asp.storage()->x;
    // 		double *x = (double*) sp->x;
    // 		for (int j=0; j<sp->ncol; ++j) {
    // 		    int l = p1[j+jj*1000];
    // 		    int colend = std::min(int(asp.storage()->ncol), j+1+jj*1000);
    // 		    for (int i=p[j]; i<p[j+1]; i++) {
    // 			// find element [k,j] in asp and check data
    // 			for (; l<p1[colend]; l++)
    // 			    if (r1[l] == r[i]+ii*1000) {
    // 				ASSERT_DOUBLE_EQ(x1[l], x[i]);
    // 				break;
    // 			    }
    // 			if (l==p1[colend])
    // 			    ASSERT_TRUE(false);
    // 		    }
    // 		}
    // 	    }
    Linearization<2> *block1 = block->transpose();
    sp.fileName = "b.bin";
    Matrix b(&sp, MDCoord<2>(cols, rows), block1);
    total = rows * cols;
    Datum_t *data = new Datum_t[total];
    for (int i=0; i<total; ++i) {
        data[i] = i;
    }
    b.batchPut(begin, MDCoord<2>(cols-1, rows-1), data);

    // // check block in b
    // for (int ii=0; ii<3; ++ii)
    //     for (int jj=0; jj<3; ++jj)
    // 	    {
    // 		Datum_t *x = new Datum_t[1000*1000];
    // 		b.batchGet(MDCoord<2>(ii*1000,jj*1000), MDCoord<2>(ii*1000+999,jj*1000+999), x);
    // 		int rowend = std::min(1000, int(cols-ii*1000));
    // 		int colend = std::min(1000, int(rows-jj*1000));
    // 		for (int i=0; i<rowend; ++i)
    // 		    for (int j=0; j<colend; ++j) {
    // 			ASSERT_DOUBLE_EQ(data[i+ii*1000+(j+jj*1000)*cols], x[i+j*1000]);
    // 		    }
    // 		delete[] x;
    // 	    }

    Matrix c = a*b;

    // // compute c[2,0] manually
    // double c20 = 0;
    // int *p = (int*) asp.storage()->p;
    // int *i = (int*) asp.storage()->i;
    // double *x = (double*) asp.storage()->x;
    // for (int j=0; j<cols; j++) {
    //     for (int k=p[j]; k<p[j+1]; k++) {
    //         if (i[k] == 2)
    //             c20 += x[k] * data[j+rows*0]; // a[2,j] * b[j,0]
    //     }
    // }
    // cout<<"c[2,0]="<<c20<<endl;

    // check correctness against in-memory cholmod_sdmult
    cholmod_dense *resultd = cholmod_allocate_dense(rows, rows, rows, CHOLMOD_REAL,
						    SparseMatrix::chmCommon);
    cholmod_dense rightd;
    rightd.nrow = cols;
    rightd.ncol = rows;
    rightd.nzmax = total;
    rightd.d = rightd.nrow;
    rightd.x = data;
    rightd.xtype = CHOLMOD_REAL;
    rightd.dtype = CHOLMOD_DOUBLE;
    double alpha[] = {1, 0};
    double beta[]  = {0, 0};
    cholmod_sdmult(
		   asp.storage(), // cholmod_sparse
		   0, // no transpose
		   alpha, // scaling factor for sparse matrix
		   beta, // scaling factor for result
		   &rightd,
		   resultd,
		   SparseMatrix::chmCommon);
    double * result = (double*) resultd->x;
    for (int i=0; i<rows; ++i) // col
        for (int j=0; j<rows; ++j) { // row
            Datum_t d;
            c.get(MDCoord<2>(j,i), d);
            ASSERT_DOUBLE_EQ(result[j+i*rows],
			     d)<<j<<"\t"<<i<<"\t"<<result[j*rows+i];
        }
    cholmod_free_dense(&resultd, SparseMatrix::chmCommon);
    delete[] elements;
    delete[] data;
    delete block;
    delete block1;
}

