
#include <math.h>
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
    for (int k=0; k<PAGE_SIZE; k+=8) {
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
    DenseArrayBlockIterator<Datum_t> *it= dab->getIterator();
    Datum_t x = 0;
    do {
        assert(**it == 8*x);
        x++;
    } while (it->next());
    delete it;

    it = dab->getIterator(PAGE_SIZE+10, PAGE_SIZE+PAGE_SIZE/8-10);
    x = 10;
    do {
        assert(**it == 8*x);
        x++;
    } while (it->next());
    delete it;

    // comparison operators
    it = dab->getIterator(PAGE_SIZE+10, PAGE_SIZE+PAGE_SIZE/8-10);
    DenseArrayBlockIterator<Datum_t> *another =
        new DenseArrayBlockIterator<Datum_t>(*it);
    x = 0;
    do {
        if (x < 256) {
            assert(*another == *it);
            it->next();
        }
        else if (x == 256)
            assert(*another == *it);
        else {
            assert(*another != *it);
        }
        x++;
    } while(another->next());
    delete it;
    delete another;

    // using comparison and increment method
    it = dab->getIterator(PAGE_SIZE+10, PAGE_SIZE+10);
    another = dab->getIterator(PAGE_SIZE+PAGE_SIZE/8-10,
            PAGE_SIZE+PAGE_SIZE/8-10);
    for (x = 10; (*it) != (*another); ++(*it), x++) {
        assert(**it == 8*x);
    }
    delete it;
    delete another;

    another = dab->getIterator(PAGE_SIZE+10, PAGE_SIZE+PAGE_SIZE/8-10);
    for (x = 10; another->hasNext(); (*another)++, x++) {
        assert(**another == 8*x);
    }
    delete another;

    delete dab;
    cout << "dense array block test cases passed" << endl;
}

void testDirectlyMappedArray() {
    int cap = PAGE_DENSE_CAP(Datum_t);
    assert(cap == 512);
    DirectlyMappedArray<Key_t, Datum_t> *dma = new DirectlyMappedArray<Key_t,
        Datum_t>("test.bin", 3*cap);
    for (int k=0; k<3*cap; k++) {
        dma->put(k, 0);
    }
 
    // test put/get
    dma->put(3, 10.2);
    dma->put(cap+10, 123);
    dma->put(2*cap, -123.456);
    assert(dma->get(3) == 10.2);
    assert(dma->get(cap+10) == 123);
    assert(dma->get(2*cap) == -123.456);
    assert(R_IsNA(dma->get(-1)));
    assert(R_IsNA(dma->get(3*cap)));
    for (int k = 0; k<3*cap; k++) {
        if (k != 3 && k!= cap+10 && k != 2*cap) {
            assert(dma->get(k) == 0);
        }
    }

    // delete and retrieve
    delete dma;
    dma = new DirectlyMappedArray<Key_t, Datum_t>("test.bin", 0);

    // test iterator
    assert(dma->get(3)==10.2);
    assert(dma->get(cap+10) == 123);
    assert(dma->get(2*cap) == -123.456);
    DirectlyMappedArrayIterator<Key_t, Datum_t> *it = dma->getIterator(1,
            2*cap+2);
    int x = 1;
    do {
        if (x == 3)
            assert(**it == 10.2);
        else if (x == cap+10)
            assert(**it == 123);
        else if (x == 2*cap)
            assert(**it == -123.456);
        else {
            assert(**it == 0);
        }
        x++; 
    } while (it->next());
    delete it;


/*
    DirectlyMappedArrayIterator<Key_t, Datum_t> *another = dma->getIterator(10, 11);
    *it = dma->getIterator(1, 10);
    assert( (*it) != (*another));
    delete it;
    it = dma->getIterator(10, 12);
    ++(*it);
    assert((*it) == (*another));
*
    it = dma->getIterator(1, 20);
    (*it)++;
    for(x = 1;it->hasNext(); (*it)++, x++) {
        if(x == 3)
            assert(**it == 10.2);
        else if(x == cap+10)
            assert(**it == 123);
        else if(x == 2*cap)
            assert(**it == -123.456);
        else 
            assert(**it == 0);
    }
    delete it;
    delete another;
*/
    delete dma;
    cout << "Directly Mapped Array test cases passed" << endl;
}

int main() {
    testDenseArrayBlock();
    testDirectlyMappedArray();
    remove("test.bin");
    return 0;
}
