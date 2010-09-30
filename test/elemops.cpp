
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "../lower/PagedStorageContainer.h"
#include "../array/MDCoord.h"
#include "../array/RowMajor.h"
#include "../array/ColMajor.h"
#include "../array/MDArray.h"

using namespace std;

int n = 300;
i64 rows = n;
i64 cols = n;
MDCoord dim(2, rows, cols);
StorageType type = DMA;
Linearization *row = new RowMajor(dim);
Linearization *col = new ColMajor(dim);
const char *fileName = "/riot/array.bin";
MDCoord coord;
Datum_t datum;

int readCount = 0;
int writeCount = 0;
double accessTime = 0.0;
double execTime = 0.0;
FILE *pFile;
const char resultFile[] = "elemops.log";

timeval tim;
double t1, t2;

/// sequential insertion into nxn array
void sequentialInsert()
{
    MDArray *array;
    Btree::MSplitter leafSp;
    Btree::MSplitter intSp;
    if (type == DMA)
        array = new MDArray(dim, type, row, fileName);
    else if (type == BTREE)
        array = new MDArray(dim, row, &leafSp, &intSp, fileName);
   PagedStorageContainer::resetPerfCounts();

   gettimeofday(&tim, NULL);
   t1 = tim.tv_sec + tim.tv_usec/1000000.0;
   for (i64 i = 0; i < n; i++)
   {
      for (i64 j = 0; j < n; j++)
      {
         coord = MDCoord(2, i, j);
         array->put(coord, 12345);
      }
   }
   delete array;

   gettimeofday(&tim, NULL);
   t2 = tim.tv_sec + tim.tv_usec/1000000.0;

   readCount += PagedStorageContainer::readCount;
   writeCount += PagedStorageContainer::writeCount;
   accessTime += PagedStorageContainer::accessTime;
   execTime += (t2 - t1);

   fprintf(pFile, "%-14s %8i %8i %11.3f %11.3f\n", "seq insert", PagedStorageContainer::readCount, PagedStorageContainer::writeCount, PagedStorageContainer::accessTime, t2-t1);
   
}

/// strided insertion into nxn array (insert going down columns first into row
//major array
void stridedInsert()
{
    MDArray *array;
    Btree::MSplitter leafSp;
    Btree::MSplitter intSp;
    if (type == DMA)
        array = new MDArray(dim, type, row, fileName);
    else if (type == BTREE)
        array = new MDArray(dim, row, &leafSp, &intSp, fileName);
   PagedStorageContainer::resetPerfCounts();

   gettimeofday(&tim, NULL);
   t1 = tim.tv_sec + tim.tv_usec/1000000.0;
   for (i64 j = 0; j < n; j++)
   {
      for (i64 i = 0; i < n; i++)
      {
         coord = MDCoord(2, i, j);
         array->put(coord, 12345);
      }
   }
   delete array;

   gettimeofday(&tim, NULL);
   t2 = tim.tv_sec + tim.tv_usec/1000000.0;
   readCount += PagedStorageContainer::readCount;
   writeCount += PagedStorageContainer::writeCount;
   accessTime += PagedStorageContainer::accessTime;
   execTime += (t2 - t1);

   fprintf(pFile, "%-14s %8i %8i %11.3f %11.3f\n", "stride insert", PagedStorageContainer::readCount, PagedStorageContainer::writeCount, PagedStorageContainer::accessTime, t2-t1);
   
}

/// strided insertion, using the iterator
void stridedIteratorInsert()
{
    MDArray *array;
    Btree::MSplitter leafSp;
    Btree::MSplitter intSp;
    if (type == DMA)
        array = new MDArray(dim, type, row, fileName);
    else if (type == BTREE)
        array = new MDArray(dim, row, &leafSp, &intSp, fileName);
   PagedStorageContainer::resetPerfCounts();
   
   gettimeofday(&tim, NULL);
   t1 = tim.tv_sec + tim.tv_usec/1000000.0;
   MDIterator *it = array->createIterator(Dense, col);
   while (it->moveNext())
   {
      it->put(12345);
   }
   delete it;
   delete array;

   gettimeofday(&tim, NULL);
   t2 = tim.tv_sec + tim.tv_usec/1000000.0;
   readCount += PagedStorageContainer::readCount;
   writeCount += PagedStorageContainer::writeCount;
   accessTime += PagedStorageContainer::accessTime;
   
   execTime += (t2 - t1);

   fprintf(pFile, "%-14s %8i %8i %11.3f %11.3f\n", "batch stride", PagedStorageContainer::readCount, PagedStorageContainer::writeCount, PagedStorageContainer::accessTime, t2-t1);

}

/// random insertion into nxn array, all indices are written to once
void randomInsert()
{
   i64 k[n*n];
   for (int i = 0; i < n*n; i++)
   {
      k[i] = i;
   }
    MDArray *array;
    Btree::MSplitter leafSp;
    Btree::MSplitter intSp;
    if (type == DMA)
        array = new MDArray(dim, type, row, fileName);
    else if (type == BTREE)
        array = new MDArray(dim, row, &leafSp, &intSp, fileName);
   PagedStorageContainer::resetPerfCounts();
   permute(k, n*n);
   i64 *i = k;

   gettimeofday(&tim, NULL);
   t1 = tim.tv_sec + tim.tv_usec/1000000.0;
   while (i < k + n*n)
   {
      coord = MDCoord(2, *i/n, *i%n);
      array->put(coord, 12345);
      i++;
   }
   delete array;

   gettimeofday(&tim, NULL);
   t2 = tim.tv_sec + tim.tv_usec/1000000.0;
   readCount += PagedStorageContainer::readCount;
   writeCount += PagedStorageContainer::writeCount;
   accessTime += PagedStorageContainer::accessTime;
   execTime += (t2 - t1);

   fprintf(pFile, "%-14s %8i %8i %11.3f %11.3f\n", "rand insert", PagedStorageContainer::readCount, PagedStorageContainer::writeCount, PagedStorageContainer::accessTime, t2-t1);


}

/// sequentially read nxn array in row major order
void sequentialRead()
{
   MDArray *array = new MDArray(fileName);
   PagedStorageContainer::resetPerfCounts();
   gettimeofday(&tim, NULL);
   t1 = tim.tv_sec + tim.tv_usec/1000000.0;
   MDIterator *it = array->createIterator(Dense, row);
   while (it->moveNext())
   {
      it->get(coord, datum);
   }
   delete it;
   delete array;

   gettimeofday(&tim, NULL);
   t2 = tim.tv_sec + tim.tv_usec/1000000.0;
   readCount += PagedStorageContainer::readCount;
   writeCount += PagedStorageContainer::writeCount;
   accessTime += PagedStorageContainer::accessTime;

   execTime += (t2 - t1);

   fprintf(pFile, "%-14s %8i %8i %11.3f %11.3f\n", "seq read", PagedStorageContainer::readCount, PagedStorageContainer::writeCount, PagedStorageContainer::accessTime, t2-t1);

}

/// random read from nxn array. all indices are read once
void randomRead()
{
   i64 k[n*n];
   for (int i = 0; i < n*n; i++)
   {
      k[i] = i;
   }
   MDArray *array = new MDArray(fileName);
   PagedStorageContainer::resetPerfCounts();
   permute(k, n*n);

   gettimeofday(&tim, NULL);
   t1 = tim.tv_sec + tim.tv_usec/1000000.0;
   for (int i = 0; i < n*n; i++)
   {
      coord = MDCoord(2, k[i]/n, k[i]%n);
      array->get(coord, datum);
   }
   delete array;

   gettimeofday(&tim, NULL);
   t2 = tim.tv_sec + tim.tv_usec/1000000.0;
   readCount += PagedStorageContainer::readCount;
   writeCount += PagedStorageContainer::writeCount;
   accessTime += PagedStorageContainer::accessTime;
   execTime += (t2 - t1);

   fprintf(pFile, "%-14s %8i %8i %11.3f %11.3f\n", "rand read", PagedStorageContainer::readCount, PagedStorageContainer::writeCount, PagedStorageContainer::accessTime, t2-t1);

}

int main()
{
    srand(time(NULL));

    MDArray::init();
   pFile = fopen(resultFile, "ab+");

   type = DMA;
   fprintf(pFile, "\nDMA, n = %-5i    reads   writes access time   exec time\n", n);
   //sequentialInsert();
   //printf("sequential insert done\n");
   /*
   stridedInsert();
   printf("strided insert done\n");
   stridedIteratorInsert();
   printf("strided iterator insert done\n");
   randomInsert();
   printf("random insert done\n");
   sequentialRead();
   printf("sequential read done\n");
   randomRead();
*/
   fprintf(pFile, "%-14s %8i %8i %11.3f %11.3f\n", "total", readCount, writeCount, accessTime, execTime);
   remove(fileName);

   // Btree
   type = BTREE;
   fprintf(pFile, "\nBTREE, n=%-5i    reads   writes access time   exec time\n", n);
   //sequentialInsert();
   //printf("sequential insert done\n");
   // stridedInsert();
   // printf("strided insert done\n");
   // stridedIteratorInsert();
   // printf("strided iterator insert done\n");
   randomInsert();
   printf("random insert done\n");
   // sequentialRead();
   // printf("sequential read done\n");
   // randomRead();
   // fprintf(pFile, "%-14s %8i %8i %11.3f %11.3f\n", "total", readCount, writeCount, accessTime, execTime);


   fclose(pFile);
   remove(fileName);
   MDArray::cleanup();
}
