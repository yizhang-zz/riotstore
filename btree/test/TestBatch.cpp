#include <gtest/gtest.h>
#include <iostream>
#include "btree/Btree.h"

using namespace std;
using namespace Btree;

//static LeafSplitter *lsp;
//static InternalSplitter *isp;

TEST(Btree, PutGet)
{
    //lsp = new MSplitter<Datum_t>();
    //isp = new MSplitter<PID_t>();

    const int rows = 210;
    const int cols = 190;
    BTree *tree = new BTree("seq.bin", rows*cols, 'B', 'M', config->useDenseLeaf);
    Datum_t datum;

    tree->put(0, 0.0);
    ASSERT_EQ(0, tree->nnz());
    tree->put(1, 1.0);
    ASSERT_EQ(1, tree->nnz());
    tree->put(1, 0.0);
    ASSERT_EQ(0, tree->nnz());

    int count = -1;
    for (int i=0; i<rows; ++i)
        for (int j=0; j<cols; ++j) {
            Key_t key = j*rows+i;
            tree->put(key, (double)key);
            ASSERT_EQ(++count, tree->nnz());
        }

    for (int i=0; i<rows; ++i)
        for (int j=0; j<cols; ++j) {
            tree->get(i*cols+j, datum);
            ASSERT_DOUBLE_EQ(i*cols+j, datum)<<"for key="<<i*cols+j<<endl;
        }
    delete tree;

    // random insertions

    tree = new BTree("rand.bin", rows*cols, 'B', 'M', config->useDenseLeaf);
    const int num = rows*cols/5;
    Key_t keys[num];
    kPermute(keys, (Key_t)1, (Key_t)rows*cols-1, num);
    count = 0;
    for (int i=0; i<num; ++i) {
        tree->put(keys[i], (Datum_t)keys[i]);
        ASSERT_EQ(++count, tree->nnz())<<"after i="<<i<<" keys[i]="<<keys[i];
    }
    for (int i=0; i<num; ++i) {
        tree->get(keys[i], datum);
        ASSERT_DOUBLE_EQ(keys[i], datum)<<" for key="<<keys[i]<<endl;
    }
    delete tree;
}

TEST(Btree, BatchPut)
{
    //lsp = new MSplitter<Datum_t>();
    //isp = new MSplitter<PID_t>();

    const int rows = 400;
    const int cols = 500;
    BTree *tree = new BTree("batch.bin", rows*cols, 'B', 'M', config->useDenseLeaf);

    const int num = 20000;
    Key_t x[num];
    Datum_t y[num];
    Entry z[num];

    kPermute(x, (Key_t)1, (Key_t)rows*cols-1, num);
    std::sort(x, x+num);
    for (int i=0; i<num; ++i) {
        y[i] = x[i];
        z[i].key = x[i];
        z[i].datum = y[i];
    }

    tree->batchPut(num, z);
    ASSERT_EQ(num, tree->nnz());

    // verify by get()
    Datum_t datum;
    for (int i=0; i<num; ++i) {
        tree->get(x[i], datum);
        ASSERT_DOUBLE_EQ(y[i], datum);
    }

    // batchGet version 1
    Datum_t w[num];
    for (int i=0; i<num; ++i)
        z[i].pdatum = w+i;
    tree->batchGet(num, z);
    for (int i=0; i<num; ++i)
        ASSERT_DOUBLE_EQ(x[i], w[i]);

    // batchGet version 2
    std::vector<Entry> v;
    tree->batchGet(0, rows*cols, v);
    for (int i=0; i<num; ++i) {
        ASSERT_EQ(x[i], v[i].key);
        ASSERT_DOUBLE_EQ(x[i], v[i].datum);
    }

    delete tree;
}
