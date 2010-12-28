#include <gtest/gtest.h>
#include "array/MDArray.h"
#include "array/RowMajor.h"

#include <iostream>
using namespace std;

TEST(MDArray, BatchLoad)
{
    const char *file = "arc130.mtx";
    i64 dims[2] = {1, 1};
    RowMajor<2> rowMajor(dims);
    StorageParam sp;
    sp.type = BTREE;
    sp.leafSp = 'B';
    sp.intSp = 'M';
    sp.fileName = "a.bin";
    sp.useDenseLeaf = config->useDenseLeaf;
    Matrix a(&sp, &rowMajor, "MM", file, 13);
    Matrix b(&sp, &rowMajor, "MM", file, 13);
    Matrix c = a * b;
    for (int i=53; i<63; ++i)
        for (int j=52; j<62; ++j) {
            Datum_t d;
            MDCoord<2> coord(i,j);
            c.get(coord, d);
            //cout<<coord<<" "<<d<<endl;
        }
}
