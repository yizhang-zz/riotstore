#include <gtest/gtest.h>
#include <iostream>
#include "btree/Btree.h"
#include "btree/Splitter.h"
#include "common/Config.h"

using namespace Btree;
using namespace std;

static LeafSplitter *lsp;
static InternalSplitter *isp;

typedef void (*SeqGenerator)(Key_t *, int);

static void genOrdered(Key_t *keys, int num)
{
	for (int i=0; i<num; ++i)
		keys[i] = i;
}

static void genRandom(Key_t *keys, int num)
{
	for (int i=0; i<num; ++i)
		keys[i] = i;
	permute(keys, num);
}

static void insert(SeqGenerator gen)
{
	int num = config->denseLeafCapacity*5;
    BTree tree("tree.bin", num, lsp, isp);
    Key_t k = 0;
    Datum_t d = 1.0;

    Key_t keys[num];
	(*gen)(keys, num);
	
    for (int i=0; i<num; ++i) {
		tree.put(keys[i], d);
    }
    tree.flushAndPrint();
}
	/*
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
	*/

TEST(BTree, RandomPut_MSplitter)
{
	lsp = new MSplitter<Datum_t>();
	isp = new MSplitter<PID_t>();
	insert(genRandom);
	delete lsp;
	delete isp;
}

TEST(BTree, InOrderPut_MSplitter)
{
	lsp = new MSplitter<Datum_t>();
	isp = new MSplitter<PID_t>();
	insert(genOrdered);
	delete lsp;
	delete isp;
}

TEST(DISABLE_BTree, InOrderPut_BSplitter)
{
#ifdef DISABLE_DENSE_LEAF
	lsp = new BSplitter<Datum_t>(config->sparseLeafCapacity);
#else
	lsp = new BSplitter<Datum_t>(config->denseLeafCapacity);
#endif
	isp = new MSplitter<PID_t>();
	insert(genOrdered);
	delete lsp;
	delete isp;
}

TEST(BTree, RandomPut_BSplitter)
{
#ifdef DISABLE_DENSE_LEAF
	lsp = new BSplitter<Datum_t>(config->sparseLeafCapacity);
#else
	lsp = new BSplitter<Datum_t>(config->denseLeafCapacity);
#endif
	isp = new MSplitter<PID_t>();
	insert(genRandom);
	delete lsp;
	delete isp;
}

TEST(DISABLE_BTree, InOrderPut_RSplitter)
{
	lsp = new RSplitter<Datum_t>();
	isp = new MSplitter<PID_t>();
	insert(genOrdered);
	delete lsp;
	delete isp;
}

TEST(BTree, RandomPut_RSplitter)
{
	lsp = new RSplitter<Datum_t>();
	isp = new MSplitter<PID_t>();
	insert(genRandom);
	delete lsp;
	delete isp;
}

TEST(BTree, RandomPut_SSplitter)
{
	lsp = new SSplitter<Datum_t>();
	isp = new MSplitter<PID_t>();
	insert(genRandom);
	delete lsp;
	delete isp;
}

TEST(BTree, RandomPut_TSplitter)
{
	lsp = new TSplitter<Datum_t>(.6);
	isp = new MSplitter<PID_t>();
	insert(genRandom);
	delete lsp;
	delete isp;
}

TEST(BTree, RandomPut_TSplitter_Read)
{
	lsp = new TSplitter<Datum_t>(.6);
	isp = new MSplitter<PID_t>();
	BTree btree("tree.bin", lsp, isp);
	btree.print();
}

