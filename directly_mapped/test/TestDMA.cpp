#include <gtest/gtest.h>
#include <math.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "../DirectlyMappedArray.h"
#include "../DMABlock.h"
#include "../DABIterator.h"
#include "../DMADenseIterator.h"
#include "../DMASparseIterator.h"
#include "../../lower/PageRec.h"
using namespace std;


TEST(DirectlyMappedArray, BatchGet)
{
    const size_t cap = DMABlock::CAPACITY;
	const Key_t upper = 3 * cap;
    DirectlyMappedArray *dma = new DirectlyMappedArray("test.bin", upper);
	// randomly generate 100 nz records
	const int num = 100;
	Key_t x[num];
	Datum_t val = 1.0;
	kPermute(x, (Key_t)0, (Key_t)upper, num);
	for (int i=0; i<num; i++)
		dma->put(x[i], val);
	std::sort(x, x+num);
	std::vector<Entry> v;
	dma->batchGet(0, upper, v);
	ASSERT_EQ(num, v.size());
	for (int i=0; i<num; i++) {
		ASSERT_EQ(x[i], v[i].key);
		ASSERT_DOUBLE_EQ(val, v[i].datum);
	}
	delete dma;
}

TEST(DirectlyMappedArray, BatchPut)
{
	const size_t cap = DMABlock::CAPACITY;
	const Key_t upper = 3 * cap;
	DirectlyMappedArray *dma = new DirectlyMappedArray("test.bin", upper);
	// randomly generate 100 nz records
	const int num = 100;
	Key_t x[num];
	Entry z[num];
	kPermute(x, (Key_t)0, (Key_t)upper, num);
	std::sort(x, x+num);
	for (int i=0; i<num; i++) {
		z[i].key = x[i];
		z[i].datum = x[i];
	}
	dma->batchPut(num, z);

    // get one by one
    Datum_t temp;
    for (int i=0; i<num; i++) {
        dma->get(x[i], temp);
        ASSERT_DOUBLE_EQ(x[i], temp);
    }

    // get in a batch
	Datum_t y[num];
	for (int i=0; i<num; i++) {
        z[i].pdatum = y+i;
    }
	dma->batchGet(num, z);
	for (int i=0; i<num; i++)
		ASSERT_DOUBLE_EQ(z[i].key, y[i]);
	delete dma;
}

// test retrieval
TEST(DirectlyMappedArray, PutGet)
{
    int cap = DMABlock::CAPACITY;
    DirectlyMappedArray *dma = new DirectlyMappedArray("test.bin", 3*cap);
    // test put/get
    Key_t k; Datum_t d;
    k = 3; d = 10.2;
    dma->put(k ,d);
    k = cap+10; d = 123;
    dma->put(k ,d);
    k = 2*cap; d = -123.456;
    dma->put(k ,d);
    k = 3;
    dma->get(k,d);
    ASSERT_DOUBLE_EQ(d, 10.2);
    k = cap+10;
    dma->get(k,d);
    ASSERT_DOUBLE_EQ(d, 123);
    k = 2*cap;
    dma->get(k,d);
    ASSERT_DOUBLE_EQ(d, -123.456);
	dma->get(5, d);
	ASSERT_DOUBLE_EQ(d, 0);

	// out of range gets should return na
    k = -1;
    dma->get(k, d);
    ASSERT_TRUE(isNA(d));
    k = 3*cap;
    dma->get(k,d);
    ASSERT_TRUE(isNA(d));
    delete dma;

	dma = new DirectlyMappedArray("test.bin", 0);
    k = 3;
    dma->get(k, d);
    ASSERT_DOUBLE_EQ(d,10.2);
    k = cap+10;
    dma->get(k,d);
    ASSERT_DOUBLE_EQ(d,123);
    k=2*cap;
    dma->get(k,d);
    ASSERT_DOUBLE_EQ(d,-123.456);
    delete dma;
    remove("test.bin");
}

TEST(DirectlyMappedArray, DenseIterator)
{
   int cap = DMABlock::CAPACITY;
   DirectlyMappedArray dma("test-di.bin", 3*cap);
   //Key_t k; Datum_t d;
   Key_t lower = 0, upper = 3*cap;
   ArrayInternalIterator *it = dma.createIterator(Dense, lower, upper);
/* unsigned count = 0;
   while (it->moveNext())
   {
      it->put(-123.456+count);
      count++;
   }

   it->reset();

   count = 0;
   while (it->moveNext())
   {
      it->get(k,d);
      ASSERT_EQ(k, count);
      ASSERT_EQ(d, -123.456+count);
      count++;
   }*/
   delete it;
   remove("test-di.bin");
}

// Incomplete
TEST(DirectlyMappedArray, SparseIterator)
{    // test iterator
   int cap = DMABlock::CAPACITY;
    DirectlyMappedArray dma("test2.bin", 3*cap);
    Key_t k; Datum_t d;
    k = 3; d = 10.2;
    dma.put(k ,d);
    k = cap+10; d = 123;
    dma.put(k ,d);
    k = 2*cap; d = -123.456;
    dma.put(k ,d);
  /*            ArrayInternalIterator *it = dma.createIterator(Sparse,0,3*cap);
(it->moveNext());
 it->get(k,d);
    ASSERT_EQ(k,3);
ASSERT_EQ(d,10.2);
    it->put(30);
    it->get(k,d);
    ASSERT_EQ(k,3);
    ASSERT_EQ(d,30);
    ASSERT_TRUE(it->moveNext());
    it->get(k,d);
    ASSERT_EQ(k,cap+10);
    ASSERT_EQ(d,123);
    ASSERT_TRUE(it->moveNext());
    it->get(k,d);
    ASSERT_EQ(k,2*cap);
    ASSERT_EQ(d,-123.456);
   ASSERT_TRUE(!it->moveNext());
  */  

    /*
    k = 3;
    dma.get(k,d);
    ASSERT_EQ(d,10.2);
    k = cap+10;
    dma.get(k,d);
    ASSERT_EQ(d , 123);
    k = 2*cap;
    dma.get(k,d);
    ASSERT_EQ(d, -123.456);
    ArrayInternalIterator *it = dma.getIterator(1, 2*cap+2);
    int x = 1;
    do {
        if (x , 3)
            ASSERT_EQ(**it , 10.2);
        else if (x , cap+10)
            ASSERT_EQ(**it , 123);
        else if (x , 2*cap)
            ASSERT_EQ(**it , -123.456);
        else {
            ASSERT_EQ(**it , 0);
        }
        x++; 
    } while (it.next());
    delete it;

*/

    remove("test2.bin");

}
/*
void testDirectlyMappedArray() {
    DirectlyMappedArrayIterator<Key_t, Datum_t> *another = dma.getIterator(10, 11);
    *it = dma.getIterator(1, 10);
    ASSERT_EQ( (*it) != (*another));
    delete it;
    it = dma.getIterator(10, 12);
    ++(*it);
    ASSERT_EQ((*it) , (*another));
*
    it = dma.getIterator(1, 20);
    (*it)++;
    for(x = 1;it.hasNext(); (*it)++, x++) {
        if(x , 3)
            ASSERT_EQ(**it , 10.2);
        else if(x , cap+10)
            ASSERT_EQ(**it , 123);
        else if(x , 2*cap)
            ASSERT_EQ(**it , -123.456);
        else 
            ASSERT_EQ(**it , 0);
    }
    delete it;
    delete another;
    delete dma;
    cout << "Directly Mapped Array test cases passed" << endl;
}

int main() {
    testDMABlock();
    testDirectlyMappedArray();
    remove("test.bin");
    return 0;
}
*/
