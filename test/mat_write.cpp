
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
    double ioTime;
    double bufferTime;
    int dma_readCount;
    int dma_writeCount;
    double dma_ioTime;

    void resetCounts()
    {
        totalTime = 0;
        readCount = 0;
        writeCount = 0;
        ioTime = 0;
        bufferTime = 0;
        dma_readCount = 0;
        dma_writeCount = 0;
        dma_ioTime = 0;
    }
};

void doArrayIterator(MDArray *array, Linearization *lin, result &r)
{
    PagedStorageContainer::resetPerfCounts();
    BufferManager::resetPerfCounts();
    LinearStorage::resetPerfCounts();
    gettimeofday(&tim, NULL);
    double start = tim.tv_sec + tim.tv_usec/1000000.0;

    MDIterator *it = array->createIterator(Dense, lin);
    while (it->moveNext())
    {
        Datum_t value = 1;
        it->put(value);
    }

    gettimeofday(&tim, NULL);
    double end = tim.tv_sec + tim.tv_usec/1000000.0;

    fprintf(pLog, "array iterator time: %f\treads: %i\twrites: %i\tI/O time: %f\tBuffer time: %f\n", end - start, PagedStorageContainer::readCount, PagedStorageContainer::writeCount, PagedStorageContainer::accessTime, BufferManager::accessTime);
    r.totalTime += (end - start);
    r.readCount += PagedStorageContainer::readCount;
    r.writeCount += PagedStorageContainer::writeCount;
    r.ioTime += PagedStorageContainer::accessTime;
    r.bufferTime += BufferManager::accessTime;
    r.dma_readCount += LinearStorage::readCount;
    r.dma_writeCount += LinearStorage::writeCount;
    r.dma_ioTime += LinearStorage::accessTime;
}

void doArrayKeyLoop(MDArray *array, result &r)
{
    Key_t size = (Key_t)array->size;
    
    PagedStorageContainer::resetPerfCounts();
    BufferManager::resetPerfCounts();
    LinearStorage::resetPerfCounts();
    gettimeofday(&tim, NULL);
    double start = tim.tv_sec + tim.tv_usec/1000000.0;

    for (Key_t key = 0; key < size; key++)
    {
        Datum_t value = 1;
        array->put(key, value);
    }

    gettimeofday(&tim, NULL);
    double end = tim.tv_sec + tim.tv_usec/1000000.0;
    
    fprintf(pLog, "array key loop time: %f\treads: %i\twrites: %i\tI/O time: %f\tBuffer time: %f\n", end - start, PagedStorageContainer::readCount, PagedStorageContainer::writeCount, PagedStorageContainer::accessTime, BufferManager::accessTime);
    r.totalTime += (end - start);
    r.readCount += PagedStorageContainer::readCount;
    r.writeCount += PagedStorageContainer::writeCount;
    r.ioTime += PagedStorageContainer::accessTime;
    r.bufferTime += BufferManager::accessTime;
    r.dma_readCount += LinearStorage::readCount;
    r.dma_writeCount += LinearStorage::writeCount;
    r.dma_ioTime += LinearStorage::accessTime;
}

void doArrayBatch(MDArray *array, MDCoord &dim, result &r)
{
    i64 rows = dim.coords[0];
    i64 cols = dim.coords[1];

    MDCoord startCoord(0, 0);
    MDCoord endCoord(rows-1, cols-1);
    Datum_t *values = new Datum_t[rows*cols];
    for (int i = 0; i < rows*cols; i++)
    {
        values[i] = 1;
    }

    PagedStorageContainer::resetPerfCounts();
    BufferManager::resetPerfCounts();
    LinearStorage::resetPerfCounts();
    gettimeofday(&tim, NULL);
    double start = tim.tv_sec + tim.tv_usec/1000000.0;

            cout << "here" << endl;
    array->batchPut(startCoord, endCoord, values);

    gettimeofday(&tim, NULL);
    double end = tim.tv_sec + tim.tv_usec/1000000.0;

    delete values;
    
    fprintf(pLog, "array coord loop time: %f\treads: %i\twrites: %i\tI/O time: %f\tBuffer time: %f\n", end - start, PagedStorageContainer::readCount, PagedStorageContainer::writeCount, PagedStorageContainer::accessTime, BufferManager::accessTime);
    r.totalTime += (end - start);
    r.readCount += PagedStorageContainer::readCount;
    r.writeCount += PagedStorageContainer::writeCount;
    r.ioTime += PagedStorageContainer::accessTime;
    r.bufferTime += BufferManager::accessTime;
    r.dma_readCount += LinearStorage::readCount;
    r.dma_writeCount += LinearStorage::writeCount;
    r.dma_ioTime += LinearStorage::accessTime;
}

void doArrayCoordLoop(MDArray *array, MDCoord &dim, result &r)
{
    i64 rows = dim.coords[0];
    i64 cols = dim.coords[1];

    PagedStorageContainer::resetPerfCounts();
    BufferManager::resetPerfCounts();
    LinearStorage::resetPerfCounts();
    gettimeofday(&tim, NULL);
    double start = tim.tv_sec + tim.tv_usec/1000000.0;

    for (i64 i = 0; i < rows; i++)
    {
        for (i64 j = 0; j < cols; j++)
        {
            MDCoord coord(2, i, j);
            Datum_t value = 1;
            array->put(coord, value);
        }
    }

    gettimeofday(&tim, NULL);
    double end = tim.tv_sec + tim.tv_usec/1000000.0;
    
    fprintf(pLog, "array coord loop time: %f\treads: %i\twrites: %i\tI/O time: %f\tBuffer time: %f\n", end - start, PagedStorageContainer::readCount, PagedStorageContainer::writeCount, PagedStorageContainer::accessTime, BufferManager::accessTime);
    r.totalTime += (end - start);
    r.readCount += PagedStorageContainer::readCount;
    r.writeCount += PagedStorageContainer::writeCount;
    r.ioTime += PagedStorageContainer::accessTime;
    r.bufferTime += BufferManager::accessTime;
    r.dma_readCount += LinearStorage::readCount;
    r.dma_writeCount += LinearStorage::writeCount;
    r.dma_ioTime += LinearStorage::accessTime;
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

/*    // doArrayBatch
    for (int i = 10; i < 101; i += 500)
    {
        result r;
        r.resetCounts();

        i64 rows = i;
        i64 cols = i;
        MDCoord dim(2, rows, cols);
        Linearization *lin = new ColMajor(dim);
        StorageType type = DMA;
        cout << "i = " << i << endl;
        fprintf(pLog, "dimensions: (%li, %li)\n", rows, cols);

        for (int j = 0; j < 10; j++)
        {
            //doMove(lin, dim);
            //doLinearize(lin, dim);

            cout << "here" << endl;
            MDArray *array = new MDArray(dim, type, lin, array_fileName); 
            cout << "here" << endl;
            doArrayBatch(array, dim, r);
            delete array;
            remove(array_fileName);
        }

        delete lin;
        fprintf(pResult, "dimensions: (%li, %li)\tcoord loop total time = %f\t reads = %i\twrites = %i\tI/O time = %f\tBuffer time = %f\n", rows, cols, r.totalTime, r.readCount, r.writeCount, r.ioTime, r.bufferTime);
        fprintf(pResult, "dma_reads = %i\tdma_writes = %i\tdma_accessTime = %f\n", r.dma_readCount, r.dma_writeCount, r.dma_ioTime);
    }
*/
  
    // doArrayIterator
    for (int i = 500; i < 501; i += 500)
    {
        result r;
        r.resetCounts();

        i64 rows = i;
        i64 cols = i;
        MDCoord dim(2, rows, cols);
        Linearization *lin = new ColMajor(dim);
        StorageType type = DMA;
        cout << "i = " << i << endl;
        fprintf(pLog, "dimensions: (%li, %li)\n", rows, cols);

        // repeat 10 times
        for (int j = 0; j < 10; j++)
        {
            //doMove(lin, dim);
            //doLinearize(lin, dim);

            MDArray *array = new MDArray(dim, type, lin, array_fileName); 
            doArrayIterator(array, new RowMajor(dim), r);
            delete array;
            remove(array_fileName);
        }

        delete lin;
        fprintf(pResult, "dimensions: (%li, %li)\titerator total time = %f\t reads = %i\twrites = %i\tI/O time = %f\tBuffer time = %f\n", rows, cols, r.totalTime, r.readCount, r.writeCount, r.ioTime, r.bufferTime);
        fprintf(pResult, "dma_reads = %i\tdma_writes = %i\tdma_accessTime = %f\n", r.dma_readCount, r.dma_writeCount, r.dma_ioTime);
    }
/*
    // doArrayKeyLoop
    for (int i = 500; i < 501; i += 500)
    {
        result r;
        r.resetCounts();

        i64 rows = i;
        i64 cols = i;
        MDCoord dim(2, rows, cols);
        Linearization *lin = new ColMajor(dim);
        StorageType type = DMA;
        cout << "i = " << i << endl;
        fprintf(pLog, "dimensions: (%li, %li)\n", rows, cols);

        for (int j = 0; j < 10; j++)
        {
            //doMove(lin, dim);
            //doLinearize(lin, dim);

            MDArray *array = new MDArray(dim, type, lin, array_fileName); 
            doArrayKeyLoop(array, r);
            delete array;
            remove(array_fileName);
        }

        delete lin;
        fprintf(pResult, "dimensions: (%li, %li)\tkey loop total time = %f\t reads = %i\twrites = %i\tI/O time = %f\tBuffer time = %f\n", rows, cols, r.totalTime, r.readCount, r.writeCount, r.ioTime, r.bufferTime);
        fprintf(pResult, "dma_reads = %i\tdma_writes = %i\tdma_accessTime = %f\n", r.dma_readCount, r.dma_writeCount, r.dma_ioTime);
    }

    // doArrayCoordLoop
    for (int i = 500; i < 501; i += 500)
    {
        result r;
        r.resetCounts();

        i64 rows = i;
        i64 cols = i;
        MDCoord dim(2, rows, cols);
        Linearization *lin = new ColMajor(dim);
        StorageType type = DMA;
        cout << "i = " << i << endl;
        fprintf(pLog, "dimensions: (%li, %li)\n", rows, cols);

        for (int j = 0; j < 10; j++)
        {
            //doMove(lin, dim);
            //doLinearize(lin, dim);

            MDArray *array = new MDArray(dim, type, lin, array_fileName); 
            doArrayCoordLoop(array, dim, r);
            delete array;
            remove(array_fileName);
        }

        delete lin;
        fprintf(pResult, "dimensions: (%li, %li)\tcoord loop total time = %f\t reads = %i\twrites = %i\tI/O time = %f\tBuffer time = %f\n", rows, cols, r.totalTime, r.readCount, r.writeCount, r.ioTime, r.bufferTime);
        fprintf(pResult, "dma_reads = %i\tdma_writes = %i\tdma_accessTime = %f\n", r.dma_readCount, r.dma_writeCount, r.dma_ioTime);
    }
*/


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
