
#include <sys/time.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "../array/MDCoord.h"
#include "../array/RowMajor.h"
#include "../array/ColMajor.h"
#include "../array/BlockBased.h"
#include "../array/MDArray.h"
#include "../array/MDIterator.h"

using namespace std;

FILE *pLog;
FILE *pResult;
const char *logFile = "log.bin";
const char *resultFile = "result.bin";
const char *array_fileName = "/var/tmp/local/array.bin";
timeval tim;

struct result
{
    double totalTime;
    int readCount;
    int writeCount;
    double accessTime;
};

void doArrayIterator(MDArray *array, Linearization *lin, result &r)
{
    PagedStorageContainer::resetPerfCounts();
    gettimeofday(&tim, NULL);
    double start = tim.tv_sec + tim.tv_usec/1000000.0;

    MDIterator *it = array->createIterator(Dense, lin);
    while (it->moveNext())
    {
        MDCoord coord;
        Datum_t value;
        it->get(coord, value);
    }

    gettimeofday(&tim, NULL);
    double end = tim.tv_sec + tim.tv_usec/1000000.0;
    int readCount = PagedStorageContainer::readCount;
    int writeCount = PagedStorageContainer::writeCount;
    double accessTime = PagedStorageContainer::accessTime;

    fprintf(pLog, "array iterator time: %f\treads: %i\twrites: %i\tI/O time: %f\n", end - start, readCount, writeCount, accessTime);
    r.totalTime += (end - start);
    r.readCount += readCount;
    r.writeCount += writeCount;
    r.accessTime += accessTime;
}

void doArrayKeyLoop(MDArray *array, result &r)
{
    Key_t size = (Key_t)array->size;
    
    PagedStorageContainer::resetPerfCounts();
    gettimeofday(&tim, NULL);
    double start = tim.tv_sec + tim.tv_usec/1000000.0;

    for (Key_t key = 0; key < size; key++)
    {
        Datum_t value;
        array->get(key, value);
    }

    gettimeofday(&tim, NULL);
    double end = tim.tv_sec + tim.tv_usec/1000000.0;
    int readCount = PagedStorageContainer::readCount;
    int writeCount = PagedStorageContainer::writeCount;
    double accessTime = PagedStorageContainer::accessTime;
    
    fprintf(pLog, "array key loop time: %f\treads: %i\twrites: %i\tI/O time: %f\n", end - start, readCount, writeCount, accessTime);
    r.totalTime += (end - start);
    r.readCount += readCount;
    r.writeCount += writeCount;
    r.accessTime += accessTime;
}

void doArrayCoordLoop(MDArray *array, MDCoord &dim, result &r)
{
    PagedStorageContainer::resetPerfCounts();
    gettimeofday(&tim, NULL);
    double start = tim.tv_sec + tim.tv_usec/1000000.0;

    for (i64 i = 0; i < dim.coords[0]; i++)
    {
        for (i64 j = 0; j < dim.coords[1]; j++)
        {
            MDCoord coord(2, i, j);
            Datum_t value;
            array->get(coord, value);
        }
    }

    gettimeofday(&tim, NULL);
    double end = tim.tv_sec + tim.tv_usec/1000000.0;
    int readCount = PagedStorageContainer::readCount;
    int writeCount = PagedStorageContainer::writeCount;
    double accessTime = PagedStorageContainer::accessTime;
    
    fprintf(pLog, "array coord loop time: %f\treads: %i\twrites: %i\tI/O time: %f\n", end - start, readCount, writeCount, accessTime);
    r.totalTime += (end - start);
    r.readCount += readCount;
    r.writeCount += writeCount;
    r.accessTime += accessTime;
}

void doMove(Linearization *lin, MDCoord &dim)
{
    i64 size = dim.coords[0]*dim.coords[1];
    i64 count = 0;
    MDCoord element(2, 0, 0);
    gettimeofday(&tim, NULL);
    double start = tim.tv_sec + tim.tv_usec/1000000.0;

    while (count < size)
    {
        element = lin->move(element, 1);
        count++;
    }

    gettimeofday(&tim, NULL);
    double end = tim.tv_sec + tim.tv_usec/1000000.0;
    fprintf(pLog, "dimensions: (%li, %li)\t move() time: %f\n", dim.coords[0], dim.coords[1], end - start);
}

void doLinearize(Linearization *lin, MDCoord &dim)
{
    i64 size = dim.coords[0]*dim.coords[1];
    i64 count = 0;
    MDCoord element(2, 0, 0);
    gettimeofday(&tim, NULL);
    double start = tim.tv_sec + tim.tv_usec/1000000.0;

    while (count < size)
    {
        element = lin->unlinearize(lin->linearize(element) + 1);
        count++;
    }

    gettimeofday(&tim, NULL);
    double end = tim.tv_sec + tim.tv_usec/1000000.0;
    fprintf(pLog, "dimensions: (%li, %li)\t unlin(lin()) time: %f\n", dim.coords[0], dim.coords[1], end - start);
}

int main()
{
    pLog = fopen(logFile, "ab+");
    pResult = fopen(resultFile, "ab+");
    MDArray *array;

    // doArrayIterator
    for (int i = 500; i < 4001; i += 500)
    {
        result r;
        r.totalTime = 0;
        r.readCount = 0;
        r.writeCount = 0;
        r.accessTime = 0;

        i64 rows = i;
        i64 cols = i;
        MDCoord dim(2, rows, cols);
        Linearization *lin = new RowMajor(dim);
        StorageType type = DMA;
        cout << "i = " << i << endl;
        fprintf(pLog, "dimensions: (%li, %li)\n", dim.coords[0], dim.coords[1]);

        array = new MDArray(dim, type, lin, array_fileName); 
        for (Key_t key = 0; key < array->size; key++)
        {
            Datum_t value = 1;
            array->put(key, value);
        }
        delete array;

        // repeat 10 times
        for (int j = 0; j < 10; j++)
        {
            //doMove(lin, dim);
            //doLinearize(lin, dim);

            array = new MDArray(array_fileName);
            doArrayIterator(array, lin, r);
            delete array;
        }

        remove(array_fileName);
        delete lin;
        fprintf(pResult, "dimensions: (%li, %li)\titerator total time = %f\t%i reads\t%i writes\t%f I/O time\n", dim.coords[0], dim.coords[1], r.totalTime, r.readCount, r.writeCount, r.accessTime);
    }

    // doArrayKeyLoop
    for (int i = 500; i < 4001; i += 500)
    {
        result r;
        r.totalTime = 0;
        r.readCount = 0;
        r.writeCount = 0;
        r.accessTime = 0;

        i64 rows = i;
        i64 cols = i;
        MDCoord dim(2, rows, cols);
        Linearization *lin = new RowMajor(dim);
        StorageType type = DMA;
        cout << "i = " << i << endl;
        fprintf(pLog, "dimensions: (%li, %li)\n", dim.coords[0], dim.coords[1]);

        array = new MDArray(dim, type, lin, array_fileName); 
        for (Key_t key = 0; key < array->size; key++)
        {
            Datum_t value = 1;
            array->put(key, value);
        }
        delete array;

        for (int j = 0; j < 10; j++)
        {
            //doMove(lin, dim);
            //doLinearize(lin, dim);

            array = new MDArray(array_fileName);
            doArrayKeyLoop(array, r);
            delete array;
        }

        remove(array_fileName);
        delete lin;
        fprintf(pResult, "dimensions: (%li, %li)\tkey loop total time = %f\t%i reads\t%i writes\t%f I/O time\n", dim.coords[0], dim.coords[1], r.totalTime, r.readCount, r.writeCount, r.accessTime);
    }

    // doArrayCoordLoop
    for (int i = 500; i < 4001; i += 500)
    {
        result r;
        r.totalTime = 0;
        r.readCount = 0;
        r.writeCount = 0;
        r.accessTime = 0;

        i64 rows = i;
        i64 cols = i;
        MDCoord dim(2, rows, cols);
        Linearization *lin = new RowMajor(dim);
        StorageType type = DMA;
        cout << "i = " << i << endl;
        fprintf(pLog, "dimensions: (%li, %li)\n", dim.coords[0], dim.coords[1]);

        array = new MDArray(dim, type, lin, array_fileName); 
        for (Key_t key = 0; key < array->size; key++)
        {
            Datum_t value = 1;
            array->put(key, value);
        }
        delete array;

        for (int j = 0; j < 10; j++)
        {
            //doMove(lin, dim);
            //doLinearize(lin, dim);

            array = new MDArray(array_fileName);
            doArrayCoordLoop(array, dim, r);
            delete array;
        }

        remove(array_fileName);
        delete lin;
        fprintf(pResult, "dimensions: (%li, %li)\tcoord loop total time = %f\t%i reads\t%i writes\t%f I/O time\n", dim.coords[0], dim.coords[1], r.totalTime, r.readCount, r.writeCount, r.accessTime);
    }
/*    for (int i = 500000; i < 4001*4001; i += 500000)
    {
        for (int j = 0; j < 10; j++)
        {
        i64 rows = i;
        MDCoord dim(2, rows, 1);
        Linearization *lin = new RowMajor(dim);
        cout << "i = " << i << endl;
        //doMove(lin, dim);
        //doLinearize(lin, dim);
        StorageType type = DMA;
        MDArray *array = new MDArray(dim, type, lin, array_fileName); 
        //doArrayIterator(array, lin);
        //doArrayLoop(array);
        doArrayNativeLoop(array);
        fprintf(pLog, "\n");
        delete lin;
        delete array;
        remove(array_fileName);
        }
    }
    for (int i = 500000; i < 4001*4001; i += 500000)
    {
        for (int j = 0; j < 10; j++)
        {
        i64 cols = i;
        MDCoord dim(2, 1, cols);
        Linearization *lin = new RowMajor(dim);
        cout << "i = " << i << endl;
        //doMove(lin, dim);
        //doLinearize(lin, dim);
        StorageType type = DMA;
        MDArray *array = new MDArray(dim, type, lin, array_fileName); 
        //doArrayIterator(array, lin);
        //doArrayLoop(array);
        doArrayNativeLoop(array);
        fprintf(pLog, "\n");
        delete lin;
        delete array;
        remove(array_fileName);
        }
    }*/
    fclose(pLog);
    fclose(pResult);
    return 0;
}
