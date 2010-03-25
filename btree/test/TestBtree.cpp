#include <gtest/gtest.h>
#include <iostream>
#include "../Btree.h"
#include "../Splitter.h"

using namespace Btree;
using namespace std;

TEST(BTree, All)
{
    MSplitter msp;
    BTree tree("tree.bin", 100, &msp, &msp);
    Key_t k = 0;
    Datum_t d = 1.0;
    int cols = 5;
    int rows = 5;
    int num = rows*cols;

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

    cols = 4;
    rows = 4;
    BTree *t2 = new BTree("t2.bin", cols*rows, &msp, &msp);
    // strided
    for (int i=0; i<rows; i++) {
        for (int j=0; j<cols; j++) {
            Key_t key = i*cols+j;
            d = key;
            t2->put(key, d);
        }
    }
    //t2->print();
    Key_t begin = 0;
    Key_t end = rows*cols;
    ArrayInternalIterator *it = t2->createIterator(Dense, begin, end);
    int i=0;
    while(it->moveNext()) {
        Key_t k;
        Datum_t d;
        it->get(k,d);
        ASSERT_DOUBLE_EQ(d, i++);
    }
    delete it;
    delete t2;

    // putting using itor
    BTree *t3 = new BTree("t3.bin", cols*rows, &msp, &msp);
    it = t3->createIterator(Dense, begin, end);
    d = 0;
    while(it->moveNext()) {
        it->put(d);
        d++;
    }

    it->reset();
    d  = 0;
    while(it->moveNext()) {
        Key_t k;
        Datum_t v;
        it->get(k,v);
        ASSERT_DOUBLE_EQ(d, v);
        d++;
    }
    delete it;
    delete t3;

    t3 = new BTree("t3.bin", &msp, &msp);
    it = t3->createIterator(Dense, begin, end);
    while(it->moveNext()) {
        Key_t k;
        Datum_t v;
        it->get(k,v);
        cout<<v<<endl;
        //ASSERT_DOUBLE_EQ(d, v);
        //d++;
    }
    delete it;
    delete t3;

    BTree *t4 = new BTree("t4.bin", 2,&msp, &msp);
    t4->put(0U, 1.0);
    delete t4;

}
