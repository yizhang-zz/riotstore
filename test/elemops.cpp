
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

int n = 100;
i64 rows = n;
i64 cols = n;
MDCoord dim(2, rows, cols);
StorageType type = DMA;
Linearization *row = new RowMajor(dim);
Linearization *col = new ColMajor(dim);
const char *fileName = "/var/tmp/local/array.bin";
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
   MDArray *array = new MDArray(dim, type, row, fileName);
   PagedStorageContainer::resetPerfCounts();

   gettimeofday(&tim, NULL);
   t1 = tim.tv_sec + tim.tv_usec/1000000.0;
   for (int i = 0; i < n; i++)
   {
      for (int j = 0; j < n; j++)
      {
         coord = MDCoord(2, i, j);
         array->put(coord, 12345);
      }
   }
   gettimeofday(&tim, NULL);
   t2 = tim.tv_sec + tim.tv_usec/1000000.0;

   readCount += PagedStorageContainer::readCount;
   writeCount += PagedStorageContainer::writeCount;
   accessTime += PagedStorageContainer::accessTime;
   execTime += (t2 - t1);

   fprintf(pFile, "%-14s %8i %8i %11.3f %11.3f\n", "seq insert", PagedStorageContainer::readCount, PagedStorageContainer::writeCount, PagedStorageContainer::accessTime, t2-t1);
   
   delete array;
}

/// strided insertion into nxn array (insert going down columns first into row
//major array
void stridedInsert()
{
   MDArray *array = new MDArray(dim, type, row, fileName);
   PagedStorageContainer::resetPerfCounts();

   gettimeofday(&tim, NULL);
   t1 = tim.tv_sec + tim.tv_usec/1000000.0;
   for (int j = 0; j < n; j++)
   {
      for (int i = 0; i < n; i++)
      {
         coord = MDCoord(2, i, j);
         array->put(coord, 12345);
      }
   }
   gettimeofday(&tim, NULL);
   t2 = tim.tv_sec + tim.tv_usec/1000000.0;
   readCount += PagedStorageContainer::readCount;
   writeCount += PagedStorageContainer::writeCount;
   accessTime += PagedStorageContainer::accessTime;
   execTime += (t2 - t1);

   fprintf(pFile, "%-14s %8i %8i %11.3f %11.3f\n", "stride insert", PagedStorageContainer::readCount, PagedStorageContainer::writeCount, PagedStorageContainer::accessTime, t2-t1);
   
   delete array;
}

/// strided insertion, using the iterator
void stridedIteratorInsert()
{
   MDArray *array = new MDArray(dim, type, row, fileName);
   PagedStorageContainer::resetPerfCounts();
   
   gettimeofday(&tim, NULL);
   t1 = tim.tv_sec + tim.tv_usec/1000000.0;
   MDIterator *it = array->createIterator(Dense, col);
   while (it->moveNext())
   {
      it->put(12345);
   }
   gettimeofday(&tim, NULL);
   t2 = tim.tv_sec + tim.tv_usec/1000000.0;
   readCount += PagedStorageContainer::readCount;
   writeCount += PagedStorageContainer::writeCount;
   accessTime += PagedStorageContainer::accessTime;
   
   execTime += (t2 - t1);

   fprintf(pFile, "%-14s %8i %8i %11.3f %11.3f\n", "batch stride", PagedStorageContainer::readCount, PagedStorageContainer::writeCount, PagedStorageContainer::accessTime, t2-t1);

   delete it;
   delete array;
}

/// random insertion into nxn array, all indices are written to once
void randomInsert()
{
   i64 k[n*n];
   for (int i = 0; i < n*n; i++)
   {
      k[i] = i;
   }
   MDArray *array = new MDArray(dim, type, row, fileName);
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
   gettimeofday(&tim, NULL);
   t2 = tim.tv_sec + tim.tv_usec/1000000.0;
   readCount += PagedStorageContainer::readCount;
   writeCount += PagedStorageContainer::writeCount;
   accessTime += PagedStorageContainer::accessTime;
   execTime += (t2 - t1);

   fprintf(pFile, "%-14s %8i %8i %11.3f %11.3f\n", "rand insert", PagedStorageContainer::readCount, PagedStorageContainer::writeCount, PagedStorageContainer::accessTime, t2-t1);


   delete array;
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
   gettimeofday(&tim, NULL);
   t2 = tim.tv_sec + tim.tv_usec/1000000.0;
   readCount += PagedStorageContainer::readCount;
   writeCount += PagedStorageContainer::writeCount;
   accessTime += PagedStorageContainer::accessTime;

   execTime += (t2 - t1);

   fprintf(pFile, "%-14s %8i %8i %11.3f %11.3f\n", "seq read", PagedStorageContainer::readCount, PagedStorageContainer::writeCount, PagedStorageContainer::accessTime, t2-t1);

   delete it;
   delete array;
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
   gettimeofday(&tim, NULL);
   t2 = tim.tv_sec + tim.tv_usec/1000000.0;
   readCount += PagedStorageContainer::readCount;
   writeCount += PagedStorageContainer::writeCount;
   accessTime += PagedStorageContainer::accessTime;
   execTime += (t2 - t1);

   fprintf(pFile, "%-14s %8i %8i %11.3f %11.3f\n", "rand read", PagedStorageContainer::readCount, PagedStorageContainer::writeCount, PagedStorageContainer::accessTime, t2-t1);

   delete array;
}

int main()
{

   pFile = fopen(resultFile, "ab+");
   fprintf(pFile, "\nn = %-10i    reads   writes access time   exec time\n", n);

   sequentialInsert();
   printf("sequential insert done\n");
   stridedInsert();
   printf("strided insert done\n");
   stridedIteratorInsert();
   printf("strided iterator insert done\n");
   randomInsert();
   printf("random insert done\n");
   sequentialRead();
   printf("sequential read done\n");
   randomRead();

   fprintf(pFile, "%-14s %8i %8i %11.3f %11.3f\n", "total", readCount, writeCount, accessTime, execTime);
   fclose(pFile);
   remove(fileName);
}
