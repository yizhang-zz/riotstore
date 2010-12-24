#include <gtest/gtest.h>
#include "array/MDArray.h"
#include "array/RowMajor.h"

#include <iostream>
using namespace std;

TEST(MDArray, BatchLoad)
{
    const char *file = "/Users/yizhang/Downloads/UFget/MM/HB/arc130/arc130.mtx";
    i64 dims[2] = {1, 1};
    RowMajor<2> rowMajor(dims);
    Matrix a("a.bin", &rowMajor, 'B', 'M', "MM", file, 13);
    Matrix b("a.bin", &rowMajor, 'B', 'M', "MM", file, 13);
    Matrix c = a * b;
    for (int i=53; i<63; ++i)
        for (int j=52; j<62; ++j) {
            Datum_t d;
            MDCoord<2> coord(i,j);
            c.get(coord, d);
            //cout<<coord<<" "<<d<<endl;
        }
}
