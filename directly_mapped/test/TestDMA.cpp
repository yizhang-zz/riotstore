
#include <iostream>
#include <string.h>
#include <assert.h>
#include "../DirectlyMappedArray.h"
using namespace std;

PageHandle pHandle;
DenseArrayBlock<Datum_t> *dab; 
Byte_t buffer[PAGE_SIZE];

void testDenseArrayBlock() {
    pHandle.pid = 123;
    for(int k=0; k<PAGE_SIZE; k+=8) {
        *((Datum_t*)(buffer+k)) = k;
    }
    pHandle.image = &buffer;
    dab = new DenseArrayBlock<Datum_t>(&pHandle, PAGE_SIZE,
            PAGE_SIZE+PAGE_SIZE/8);

    assert(dab->getLowerBound() == PAGE_SIZE);
    assert(dab->getUpperBound() == PAGE_SIZE+PAGE_SIZE/8);
    assert(dab->getPID() == 123);

    dab->setRange(20, 34);
    assert(dab->getLowerBound() == 20);
    assert(dab->getUpperBound() == 34);

    dab->setUpperBound(10);
    assert(dab->getUpperBound() == 10);
    dab->setLowerBound(12345);
    assert(dab->getLowerBound() == 12345);

    // test get
    for (int k=0; k<PAGE_SIZE/8; k++) {
        Key_t key = dab->getLowerBound() + k;
        assert(dab->get(key) == 8*k); 
    }

    // test put
    dab->setRange(PAGE_SIZE, PAGE_SIZE+PAGE_SIZE/8);
    dab->put(PAGE_SIZE+124, -123.456);
    assert(dab->get(PAGE_SIZE+124) == -123.456);
    
    // test iterator
    dab->put(PAGE_SIZE+124, 124*8);
    DenseArrayBlockIterator<Datum_t> it= dab->getIterator();
    Datum_t x = 0;
    while(it.next()) {
        assert(*it == 8*x);
        it++;
        x++;
    }

    it = dab->getIterator(PAGE_SIZE+10, PAGE_SIZE+PAGE_SIZE/8-10);
    x = 10;
    while(it.next()) {
        assert(*it == 8*x);
        ++it;
        x++;
    }

    it = dab->getIterator(PAGE_SIZE+10, PAGE_SIZE+PAGE_SIZE/8-10);
    DenseArrayBlockIterator<Datum_t> another = DenseArrayBlockIterator<Datum_t>(it);
    x = 0;
    while(another.next()) {
        if(x < 10) {
            assert(another != it);
        }
        else if (x < PAGE_SIZE/8-10) {
            assert(another == it);
        }
        else {
            assert(another != it);
        }
        x++;
        it++;
    }


    delete dab;
    cout << "dense array block test cases passed" << endl;
}

void testDirectlyMappedArray() {
    DirectlyMappedArray<Key_t, Datum_t> *dma = new DirectlyMappedArray<Key_t,
        Datum_t>("test.bin", 3*PAGE_SIZE);
/*    for(int k=0; k<4*PAGE_SIZE; k++) {
        dma->put(k, (double)k/PAGE_SIZE);
    }
    for(int k=0; k<4*PAGE_SIZE; k++) {
        assert(dma->get(k) == (double)k/PAGE_SIZE);
    }
  */
    dma->put(3, 10.2);
    dma->put(PAGE_SIZE+10, 123);
    dma->put(2*PAGE_SIZE, -123.456);
    assert(dma->get(3) == 10.2);
    assert(dma->get(PAGE_SIZE+10) == 123);
    assert(dma->get(2*PAGE_SIZE) == -123.456);
    assert(R_IsNA(dma->get(-1)));
    assert(R_IsNA(dma->get(3*PAGE_SIZE)));
    for(unsigned k = 0; k<3*PAGE_SIZE; k++) {
        if(dma->get(k) != 0) {
            cout << "non-zero value at: " << endl;
            cout << k<< "\t"<<(dma->get(k)) << endl;
        }
    }

//    DirectlyMappedArrayIterator<Key_t, Datum_t> *it = dma->getIterator(0,
  //          4*PAGE_SIZE);

    cout << "Directly Mapped Array test cases passed" << endl;
}

int main() {
    remove("test.bin");
//    testDenseArrayBlock();
    testDirectlyMappedArray();
    return 0;
}
