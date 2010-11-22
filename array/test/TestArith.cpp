#include <gtest/gtest.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
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
	i64 arrayDims[] = {3L, 3L};
	i64 blockDims[] = {2L, 2L};
	u8  orders[] = {0,1};
	i64 rows = arrayDims[0];
	i64 cols = arrayDims[1];
	BlockBased<2> *block = new BlockBased<2>(arrayDims, blockDims, orders, orders);
	Matrix a(MDCoord<2>(arrayDims), block, NULL, NULL, "1.bin");

	i64 total = rows*cols;
	Datum_t *initial = new Datum_t[total];
	for (int i=0; i<total; ++i)
		initial[i] = i+1;

	a.batchPut(MDCoord<2>(0,0), MDCoord<2>(rows-1, cols-1), initial);
	printMDArray(a);

	Matrix b(MDCoord<2>(arrayDims), block, NULL, NULL, "1.bin");
	b.batchPut(MDCoord<2>(0,0), MDCoord<2>(rows-1, cols-1), initial);
	a += b;
	cout<<"--- after addition ---"<<endl;
	printMDArray(a);

	cout<<"--- after multiplication ---"<<endl;
	Matrix c(a*a);
	printMDArray(c);
}
