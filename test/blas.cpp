
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "../array/MDCoord.h"
#include "../array/RowMajor.h"
#include "../array/ColMajor.h"
#include "../array/BlockBased.h"
#include "../array/MDArray.h"
#include <cblas.h>

using namespace std;

#define n 300L
#define blk_size 30L

i64 sparsity = 50L; // 50% sparsity

int readCount = 0;
int writeCount = 0;
double accessTime = 0.0;
double execTime = 0.0;
double blasTime = 0.0;
FILE *pFile;
const char *resultFile = "blas.bin";

timeval tim;
double t1, t2;
double t3, t4;

i64 rows = n;
i64 cols = n;
MDCoord matrix_dim(2, rows, cols);
MDCoord vector_dim(2, rows, 1);
StorageType type = DMA;
Linearization *matrix_row = new RowMajor(matrix_dim);
Linearization *vector_row = new RowMajor(vector_dim);
const char *A_fileName = "/var/tmp/local/A.bin";
const char *B_fileName = "/var/tmp/local/B.bin";
const char *C_fileName = "/var/tmp/local/C.bin";
const char *x_fileName = "/var/tmp/local/x.bin";

MDCoord coord;

/// C = A*x, where A is nxn matrix in row major order, x is nx1 vector, and C is resulting nx1
//vector. x is read in and stored in memory, while A is streamed in 1 row at a
//time to compute 1 value of C at a time.
void matVecMult()
{
   blasTime = 0;
   double m[n];
   double v[n];
   double y;
   MDArray *A = new MDArray(A_fileName);
   MDArray *x = new MDArray(x_fileName);
   MDArray *C = new MDArray(vector_dim, type, vector_row, C_fileName);
   PagedStorageContainer::resetCounts();

   gettimeofday(&tim, NULL);
   t1 = tim.tv_sec + tim.tv_usec/1000000.0;
   for (int i = 0; i < n; i++)
   {
      coord = MDCoord(2, i, 0);
      x->get(coord, v[i]);
   }
   for (int i = 0; i < n; i++)
   {
      for (int j = 0; j < n; j++)
      {
         coord = MDCoord(2, i, j);
         A->get(coord, m[j]);
      }
      gettimeofday(&tim, NULL);
      t3 = tim.tv_sec + tim.tv_usec/1000000.0;
      cblas_dgemv(CblasRowMajor, CblasNoTrans, 1, n, 1.0, m, n, v, 1, 0.0, &y, 1);
      gettimeofday(&tim, NULL);
      t4 = tim.tv_sec + tim.tv_usec/1000000.0;
      blasTime += (t4 - t3);
      coord = MDCoord(2, i, 0);
      C->put(coord, y);
   }
   gettimeofday(&tim, NULL);
   t2 = tim.tv_sec + tim.tv_usec/1000000.0;
   execTime += (t2 - t1);

   readCount += PagedStorageContainer::readCount;
   writeCount += PagedStorageContainer::writeCount;
   accessTime += PagedStorageContainer::accessTime;

   fprintf(pFile, "mat-vec multiplication\t\t%i\t\t%i\t\t%f\t\t%f\t\t%f\n", PagedStorageContainer::readCount, PagedStorageContainer::writeCount, PagedStorageContainer::accessTime, t2-t1, blasTime);

   delete A;
   delete x;
   delete C;
}

/// C = A*B for A, B, and C nxn matrices in row major order. for each column in
//B, iterate through each row in A and store the result in C.
void matMatMultRowMajor()
{
   blasTime = 0;
   double m[n];
   double v[n];
   double y;
   MDArray *A = new MDArray(A_fileName);
   MDArray *B = new MDArray(B_fileName);
   MDArray *C = new MDArray(matrix_dim, type, matrix_row, C_fileName);
   PagedStorageContainer::resetCounts();

   gettimeofday(&tim, NULL);
   t1 = tim.tv_sec + tim.tv_usec/1000000.0;
   for (int k = 0; k < n; k++)
   {
      for (int l = 0; l < n; l++)
      {
         coord = MDCoord(2, l, k);
         B->get(coord, v[l]);
      }
      for (int i = 0; i < n; i++)
      {
         for (int j = 0; j < n; j++)
         {
            coord = MDCoord(2, i, j);
            A->get(coord, m[j]);
         }
         gettimeofday(&tim, NULL);
         t3 = tim.tv_sec + tim.tv_usec/1000000.0;
         cblas_dgemv(CblasRowMajor, CblasNoTrans, 1, n, 1.0, m, n, v, 1, 0.0, &y, 1);
         gettimeofday(&tim, NULL);
         t4 = tim.tv_sec + tim.tv_usec/1000000.0;
         blasTime += (t4 - t3);
         coord = MDCoord(2, i, k);
         C->put(coord, y);
      }
   }
   gettimeofday(&tim, NULL);
   t2 = tim.tv_sec + tim.tv_usec/1000000.0;
   execTime += (t2 - t1);

   readCount += PagedStorageContainer::readCount;
   writeCount += PagedStorageContainer::writeCount;
   accessTime += PagedStorageContainer::accessTime;

   fprintf(pFile, "mat-mat multiplication\t\t%i\t\t%i\t\t%f\t\t%f\t\t%f\n", PagedStorageContainer::readCount, PagedStorageContainer::writeCount, PagedStorageContainer::accessTime, t2-t1, blasTime);

   delete A;
   delete B;
   delete C;
}

/// C = A*B where A, B, and C are nxn matrices, all in block order. result in C
//is computed 1 block at a time; for each block of C currently held in memory,
//iterate through row of A and column of B that contributes to the result in C,
//do the appropriate multiplication, and save intermediate result in C.
void matMatMultBlock()
{
   blasTime = 0;
   double m[blk_size*blk_size];
   double v[blk_size*blk_size];
   double y[blk_size*blk_size];

   MDArray *A = new MDArray(A_fileName);
   MDArray *B = new MDArray(B_fileName);
   MDArray *C = new MDArray(C_fileName);
   PagedStorageContainer::resetCounts();

   Datum_t temp;
   gettimeofday(&tim, NULL);
   t1 = tim.tv_sec + tim.tv_usec/1000000.0;
   
   for (int i = 0; i < n/blk_size; i++)
   {
      for (int j = 0; j < n/blk_size; j++)
      {
         //Block A = 30*i+j;
         //Block B = i+30*j;
         for (int l = 0; l < n/blk_size; l++)
         {
            for (int k = 0; k < blk_size*blk_size; k++)
            {
               coord = MDCoord(2, blk_size*i + k/blk_size, blk_size*l + k%blk_size);
               A->get(coord, m[k]);
               coord = MDCoord(2, blk_size*l + k/blk_size, blk_size*j + k%blk_size);
               B->get(coord, v[k]);
            }
            memset(y, 0, blk_size*blk_size*sizeof(double));
            gettimeofday(&tim, NULL);
            t3 = tim.tv_sec + tim.tv_usec/1000000.0;
            cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, blk_size, blk_size, blk_size, 1.0, m, blk_size, v, blk_size, 0.0, y, blk_size);
            gettimeofday(&tim, NULL);
            t4 = tim.tv_sec + tim.tv_usec/1000000.0;
            blasTime += (t4 - t3);
            for (int k = 0; k < blk_size*blk_size; k++)
            {
               coord = MDCoord(2, blk_size*i + k/blk_size, blk_size*j + k%blk_size);
               C->get(coord, temp);
               temp += y[k];
               C->put(coord, temp);
            }
         }
      }
   }
   gettimeofday(&tim, NULL);
   t2 = tim.tv_sec + tim.tv_usec/1000000.0;
   execTime += (t2 - t1);

   readCount += PagedStorageContainer::readCount;
   writeCount += PagedStorageContainer::writeCount;
   accessTime += PagedStorageContainer::accessTime;

   fprintf(pFile, "mat-mat multiplication\t\t%i\t\t%i\t\t%f\t\t%f\t\t%f\n", PagedStorageContainer::readCount, PagedStorageContainer::writeCount, PagedStorageContainer::accessTime, t2-t1, blasTime);

   delete A;
   delete B;
   delete C;
}

int main()
{

   pFile = fopen(resultFile, "ab+");
   fprintf(pFile, "\nn = %li\t\t\treads\t\twrites\t\taccess time\t\texecution time\n", n);

   i64 k[n*n];
   for (int i = 0; i < n*n; i++)
   {
      k[i] = i;
   }

   permute(k, n*n);
   MDArray *A = new MDArray(matrix_dim, type, matrix_row, A_fileName);
   for (int i = 0; i < n*n; i++)
   {
      coord = MDCoord(2, i/n, i%n);
      A->put(coord, 0);
   }
   for (int i = 0; i < n*n*sparsity/100; i++)
   {
      coord = MDCoord(2, k[i]/n, k[i]%n);
      A->put(coord, i);
   }
   delete A;

   permute(k, n*n);
   MDArray *B = new MDArray(matrix_dim, type, matrix_row, B_fileName);
   for (int i = 0; i < n*n; i++)
   {
      coord = MDCoord(2, i/n, i%n);
      B->put(coord, 0);
   }
   for (int i = 0; i < n*n*sparsity/100; i++)
   {
      coord = MDCoord(2, k[i]/n, k[i]%n);
      B->put(coord, i);
   }
   delete B;

   MDArray *x = new MDArray(vector_dim, type, vector_row, x_fileName);
   for (int i = 0; i < n; i++)
   {
      coord = MDCoord(2, i, 0);
      x->put(coord, i*i+1);
   }
   delete x;

   matVecMult();
   matMatMultRowMajor();

   i64 blockDims[] = {blk_size, blk_size};
   u8 blockOrders[] = {0, 1};
   u8 microOrders[] = {0, 1};
   Linearization *block = new BlockBased(2, matrix_dim.coords, blockDims, blockOrders, microOrders);

   permute(k, n*n);
   A = new MDArray(matrix_dim, type, block, A_fileName);
   for (int i = 0; i < n*n; i++)
   {
      coord = MDCoord(2, i/n, i%n);
      A->put(coord, 0);
   }
   for (int i = 0; i < n*n*sparsity/100; i++)
   {
      coord = MDCoord(2, k[i]/n, k[i]%n);
      A->put(coord, i);
   }
   delete A;

   permute(k, n*n);
   B = new MDArray(matrix_dim, type, block, B_fileName);
   for (int i = 0; i < n*n; i++)
   {
      coord = MDCoord(2, i/n, i%n);
      B->put(coord, 0);
   }
   for (int i = 0; i < n*n*sparsity/100; i++)
   {
      coord = MDCoord(2, k[i]/n, k[i]%n);
      B->put(coord, i);
   }
   delete B;

   MDArray *C = new MDArray(matrix_dim, type, block, C_fileName);
   for (int i = 0; i < n*n; i++)
   {
      coord = MDCoord(2, i/n, i%n);
      C->put(coord, 0);
   }
   delete C;

   matMatMultBlock();

   fprintf(pFile, "total\t\t\t%i\t\t%i\t\t%f\t\t%f\n\n", readCount, writeCount, accessTime, execTime);
   fclose(pFile);
   remove(A_fileName);
   remove(B_fileName);
   remove(C_fileName);
   remove(x_fileName);
}
