#include <gtest/gtest.h>
#include "array/MDArray.h"
#include "array/RowMajor.h"
#include "btree/Btree.h"

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
    ((Btree::BTree*)a.storage)->print(true);
}
