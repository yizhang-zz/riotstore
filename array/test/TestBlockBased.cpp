#include <gtest/gtest.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <map>

#include "../RowMajor.h"
#include "../ColMajor.h"
#include "../MDCoord.h"
#include "../BlockBased.h"
#include "../MDArray.h"

using namespace std;

TEST(BlockBased, Create)
{
/*    i64 rows = 3L;
    i64 cols = 8L;
    MDCoord dim(2, rows, cols);
    i64 blockDims[] = {2L, 5L};
    u8 blockOrders[] = {0, 1};
    u8 microOrders[] = {0, 1};
    Linearization *block = new BlockBased(dim.nDim, dim.coords, blockDims,
                                          blockOrders, microOrders);
    MDCoord t(2,2,3);
    ASSERT_EQ(19,block->linearize(t));
    t = MDCoord(2,1,5);
    ASSERT_EQ(13,block->linearize(t));
*/
    i64 rows = 9L;
    i64 cols = 9L;
    MDCoord dim(2, rows, cols);
    i64 blockDims[] = {4L, 2L};
    u8 blockOrders[] = {0, 1};
    u8 microOrders[] = {0, 1};
    BlockBased *block = new BlockBased(dim.nDim, dim.coords, blockDims,
                                          blockOrders, microOrders);
    for (int i=0; i<rows; i++) {
        for (int j=0; j<cols; j++) {
            MDCoord c(2,i,j);
            cout<<block->linearize(c)<<"\t";
           }
        cout<<endl;
    }

    for (int i=0; i<rows; i++) {
        for (int j=0; j<cols; j++) {
            MDCoord c(2,i,j);
            ASSERT_TRUE(block->unlinearize(block->linearize(c))==c);
           }
    }

    Key_t b, e;
    block->getBlock(0,b,e);
    cout<<b<<"\t"<<e<<endl;
    block->getBlock(18,b,e);
    cout<<b<<"\t"<<e<<endl;
    block->getBlock(29,b,e);
    cout<<b<<"\t"<<e<<endl;

    cout<<block->linearize(MDCoord(2,9,9))<<endl;
}
