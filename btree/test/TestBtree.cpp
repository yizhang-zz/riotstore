#include <gtest/gtest.h>
#include "../Btree.h"
#include "../Splitter.h"

using namespace Btree;

TEST(BTree, All)
{
    MSplitter msp;
    BTree tree("tree.bin", 100, &msp, &msp);
    Key_t k = 0;
    Datum_t d = 1.0;
    const int cols = 5;
    const int rows = 5;
    const int num = rows*cols;

    //Key_t keys[] = {8,5,0,3,2,4,6,7,9,1,10,13,12,11};
    Key_t keys[num];
    for (int i=0; i<num; i++)
        keys[i] = i;
    // sequential
    for (int i=0; i<rows; i++) {
        for (int j=0; j<cols; j++) {
            Key_t key = i*cols+j;
            tree.put(key, d);
        }
    }
    tree.print();

    BTree *t2 = new BTree("t2.bin", 100, &msp, &msp);
    // strided
    for (int i=0; i<rows; i++) {
        for (int j=0; j<cols; j++) {
            Key_t key = i+j*rows;
            t2->put(key, d);
        }
    }
    t2->print();
    delete t2;
}
