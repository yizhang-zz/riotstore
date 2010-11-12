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

TEST(RowMajor, Linearize)
{
	i64 arrayDims[] = {5L, 7L};
	i64 rows = arrayDims[0];
	i64 cols = arrayDims[1];
    RowMajor<2> *block = new RowMajor<2>(arrayDims);
    for (int i=0; i<rows; i++) {
        for (int j=0; j<cols; j++) {
            MDCoord<2> c(i,j);
            cout<<block->linearize(c)<<"\t";
           }
        cout<<endl;
    }
}

TEST(RowMajor, Unlinearize)
{
	i64 arrayDims[] = {5L, 7L};
	i64 rows = arrayDims[0];
	i64 cols = arrayDims[1];
    RowMajor<2> *block = new RowMajor<2>(arrayDims);
	i64 count = rows*cols;
    for (i64 i=0; i<count; i++) {
            cout<<block->unlinearize(i).toString()<<"\t";
	}
	cout<<endl;
}

TEST(RowMajor, Inverse)
{
	i64 arrayDims[] = {5L, 7L};
	i64 rows = arrayDims[0];
	i64 cols = arrayDims[1];
    RowMajor<2> *block = new RowMajor<2>(arrayDims);
    for (int i=0; i<rows; i++) {
        for (int j=0; j<cols; j++) {
            MDCoord<2> c(i,j);
            ASSERT_TRUE(block->unlinearize(block->linearize(c))==c);
           }
    }
}

TEST(RowMajor, Move)
{

	i64 arrayDims[] = {15L, 17L};
	i64 rows = arrayDims[0];
	i64 cols = arrayDims[1];
    RowMajor<2> *block = new RowMajor<2>(arrayDims);
    for (int i=0; i<rows; i++) {
        for (int j=0; j<cols; j++) {
            MDCoord<2> c(i,j);
			Key_t kc = block->linearize(c);
			for (int u=0; u<rows; u++) {
				for (int v=0; v<cols; v++) {
					MDCoord<2> d(u,v);
					Key_t kd = block->linearize(d);
					KeyDiff_t diff = kd-kc;
					ASSERT_EQ(d, block->move(c, diff));
				}
			}
		}
    }
}
