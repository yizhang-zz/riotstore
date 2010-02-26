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

TEST(MDArray, Create)
{
    i64 rows = 500L;
    i64 cols = 500L;
    MDCoord dim(2, rows, cols);
    StorageType type = DMA;
    Linearization *row = new RowMajor(dim);
    Linearization *col = new ColMajor(dim);
    i64 blockDims[] = {100L, 100L};
    u8 blockOrders[] = {0, 1};
    u8 microOrders[] = {0, 1};
    permute(blockOrders, dim.nDim);
    permute(microOrders, dim.nDim);
    Linearization *block = new BlockBased(dim.nDim, dim.coords, blockDims,
                                          blockOrders, microOrders);
    const char *fileName = "test.bin";

    MDArray *array = new MDArray(dim, type, row, fileName);
    MDCoord coord1 = MDCoord(2, 120, 19);
    ASSERT_TRUE(AC_OK == array->put(coord1, 12345));
    Datum_t datum;
    ASSERT_TRUE(AC_OK == array->get(coord1, datum));
    ASSERT_DOUBLE_EQ(12345, datum);

    MDCoord coord2 = MDCoord(2, 400, 133);
    ASSERT_TRUE(AC_OK == array->put(coord2, 12.345));
    MDCoord coord3 = MDCoord(2, 256, 19);
    ASSERT_TRUE(AC_OK == array->put(coord3, 1000));
    ASSERT_TRUE(AC_OK == array->get(coord2, datum));
    ASSERT_DOUBLE_EQ(12.345, datum);
    ASSERT_TRUE(AC_OK == array->get(coord3, datum));
    ASSERT_DOUBLE_EQ(1000, datum);

    // dense itor, write
    MDIterator *it = array->createIterator(Dense, row);
    MDCoord coord;
    int i = 0;
    while (it->moveNext()) 
    {
        it->put(i);
        i++;
    } 
    ASSERT_EQ(rows*cols, i);
    delete it;

    // block itor, read
    it = array->createIterator(Dense, block);
    i = 0;
    while(it->moveNext())
    {
        it->get(coord, datum);
        ASSERT_DOUBLE_EQ(row->linearize(coord), datum);
        i++;
    }
    ASSERT_EQ(rows*cols, i);
    delete it;

    /*
    // natural itor, read
    it = array->createNaturalIterator(Dense);
    i = 1;
    while(it->moveNext())
    {
        it->get(coord, datum);
        ASSERT_EQ(i, row->linearize(coord));
        ASSERT_EQ(i, datum);
        i++;
    }
    ASSERT_EQ(rows*cols, i);
    delete it;
    */

    delete row;
    delete col;
    delete block;
    delete array;
}

TEST(MDArray, GenerateData)
{
    i64 rows = 20L;
    i64 cols = 20L;
    MDCoord dim(2, rows, cols);
    StorageType type = DMA;
    i64 blockDims[] = {10L, 10L};
    u8 blockOrders[] = {0, 1};
    u8 microOrders[] = {0, 1};
    //permute(blockOrders, dim.nDim);
    //permute(microOrders, dim.nDim);
    Linearization *block = new BlockBased(dim.nDim, dim.coords, blockDims,
                                          blockOrders, microOrders);
    const char *fileName = "a.bin";
    MDCoord key;
    Datum_t datum = 0.0;

    MDArray *array = new MDArray(dim, type, block, fileName);
    
    MDIterator *it = array->createIterator(Dense, block);

    while(it->moveNext()) {
        it->put(datum);
        datum += 1;
    }
    
    delete it;

    delete array;

    array = new MDArray(fileName);
    it = array->createIterator(Dense, block);
    while(it->moveNext()) {
        it->get(key, datum);
        cout<<"key "<<key.toString()<<"\t datum "<<datum<<endl;
    }
    delete it;
    delete array;
    
    delete block;
}


TEST(MDArray, Read)
{
    i64 rows = 20L;
    i64 cols = 20L;
    MDCoord dim(2, rows, cols);
    StorageType type = DMA;
    i64 blockDims[] = {10L, 10L};
    u8 blockOrders[] = {0, 1};
    u8 microOrders[] = {0, 1};
    //permute(blockOrders, dim.nDim);
    //permute(microOrders, dim.nDim);
    Linearization *block = new BlockBased(dim.nDim, dim.coords, blockDims,
                                          blockOrders, microOrders);
    const char *fileName = "test1.bin";
    MDCoord key;
    Datum_t datum = 1.0;

    MDArray *array = new MDArray(dim, type, block, fileName);
    
    MDIterator *it = array->createIterator(Dense, block);

    srand(time(NULL));
    const int num=100;
    std::map<int, MDCoord> keys;
    std::map<int, Datum_t> data;
    int total = rows*cols;
    for(int i=0; i<num; i++) {
        int k = rand() % total;
        if (data.find(k) == data.end()) {
            data[k] = rand();
            it->setIndexRange(k, k+1);
            it->moveNext();
            it->put(data[k]);
            MDCoord key;
            it->get(key, datum);
            keys[k] = key;
        }
        else
            i--;
    }
    
    delete it;

    delete array;

    array = new MDArray(fileName);
    Datum_t datum1;
    for (std::map<int,MDCoord>::iterator s = keys.begin();
         s != keys.end();
         ++s) {
        //cout<<"checking "<<s->second.toString()<<endl;
        array->get(s->second, datum1);
        ASSERT_DOUBLE_EQ(data[s->first], datum1);
    }
    delete array;
    
    delete block;
}
