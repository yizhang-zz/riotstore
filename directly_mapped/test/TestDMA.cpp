#include <gtest/gtest.h>
#include <math.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "../DirectlyMappedArray.h"
#include "../DenseArrayBlock.h"
#include "../DABIterator.h"
#include "../DMADenseIterator.h"
#include "../DMASparseIterator.h"
#include "../../lower/PageRec.h"
using namespace std;

/*
TEST(DenseArrayBlock, DISABLED_Init)
{
    PageRec rec;
    rec.pid = 123;
    rec.image = new char[PAGE_SIZE];
    PageHandle ph = &rec;

   DenseArrayBlock dab (ph, PAGE_SIZE, PAGE_SIZE+PAGE_SIZE/8);

   for (int k=0; k<PAGE_SIZE; k+=8) {
      image[k] = k;
   }

   ASSERT_EQ(dab.getLowerBound() , PAGE_SIZE);
   ASSERT_EQ(dab.getUpperBound() , PAGE_SIZE+PAGE_SIZE/8);
   ASSERT_EQ(dab.getPID() , 123);

   dab.setRange(20, 34);
   ASSERT_EQ(dab.getLowerBound() , 20);
   ASSERT_EQ(dab.getUpperBound() , 34);

   dab.setUpperBound(10);
   ASSERT_EQ(dab.getUpperBound() , 10);
   dab.setLowerBound(12345);
   ASSERT_EQ(dab.getLowerBound() , 12345);
}

TEST(DenseArrayBlock, DISABLED_PutGet)
{   
   PageImage image;
   PageHandle ph;
   ph.image = &image;
   ph.pid = 123;
   DenseArrayBlock dab (&ph, PAGE_SIZE, PAGE_SIZE+PAGE_SIZE/8);
    dab.setRange(PAGE_SIZE, PAGE_SIZE+PAGE_SIZE/8);
    for (int k =PAGE_SIZE; k<PAGE_SIZE+PAGE_SIZE/8; k++)
       dab.put(k, -123.456+k);
    for (int k =PAGE_SIZE; k<PAGE_SIZE+PAGE_SIZE/8; k++)
       ASSERT_EQ(dab.get(k) , -123.456+k);
}

TEST(DenseArrayBlock, DISABLED_Iterator)
{   // test iterator
    PageRec rec;
    rec.pid = 123;
    rec.image = new char[PAGE_SIZE];
    PageHandle ph = &rec;

   DenseArrayBlock dab (&ph, PAGE_SIZE, PAGE_SIZE+PAGE_SIZE/8);
    dab.setRange(PAGE_SIZE, PAGE_SIZE+PAGE_SIZE/8);
    ArrayInternalIterator *it= dab.getIterator();

    int k = 0;
    while (it->moveNext())
    {
       it->put(-123.456+k);
       k++;
    }

    it->reset();

    k = 0;
    Key_t key; Datum_t d;
    while (it->moveNext())
    {
      it->get(key, d);
      ASSERT_EQ(key, PAGE_SIZE+k);
      ASSERT_EQ(d, -123.456+k);
      k++;
    }
    delete it;
}
*/
// test retrieval
TEST(DirectlyMappedArray, PutGet)
{
    int cap = DenseArrayBlock::CAPACITY;
    ASSERT_EQ(cap , 512);
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
    ASSERT_EQ(d, 10.2);
    k = cap+10;
    dma->get(k,d);
    ASSERT_EQ(d, 123);
    k = 2*cap;
    dma->get(k,d);
    ASSERT_EQ(d, -123.456);
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
    ASSERT_EQ(d,10.2);
    k = cap+10;
    dma->get(k,d);
    ASSERT_EQ(d,123);
    k=2*cap;
    dma->get(k,d);
    ASSERT_EQ(d,-123.456);
    delete dma;
    /*for (int k = 0; k<3*cap; k++) {
        if (k != 3 && k!= cap+10 && k != 2*cap) {
            ASSERT_EQ(dma.get(k) , 0);
        }
    }*/

    remove("test.bin");

}

TEST(DirectlyMappedArray, DenseIterator)
{
   int cap = DenseArrayBlock::CAPACITY;
   DirectlyMappedArray dma("test-di.bin", 3*cap);
   Key_t k; Datum_t d;
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
   int cap = DenseArrayBlock::CAPACITY;
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
    testDenseArrayBlock();
    testDirectlyMappedArray();
    remove("test.bin");
    return 0;
}
*/
