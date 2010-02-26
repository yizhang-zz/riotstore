
#include <gtest/gtest.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <time.h>
#include "../RowMajor.h"
#include "../ColMajor.h"
#include "../MDCoord.h"
#include "../BlockBased.h"
#include "../MDArray.h"

using namespace std;

TEST(MDCoord, Ctor)
{
   MDCoord m1;
   ASSERT_EQ(0, m1.nDim);
   ASSERT_TRUE(m1.coords == NULL);

   MDCoord m2(0);
   ASSERT_EQ(0, m2.nDim);
   ASSERT_TRUE(m2.coords == NULL);

   // test for negative indices
   MDCoord m3(3, COORD(-1), COORD(12), COORD(123));
   ASSERT_EQ(3, m3.nDim);
   ASSERT_EQ(-1, m3.coords[0]);
   ASSERT_EQ(12, m3.coords[1]);
   ASSERT_EQ(123, m3.coords[2]);

   i64 coords[] = {1, -12, 123};
   MDCoord m4(coords, 3);
   ASSERT_EQ(3, m4.nDim);
   ASSERT_EQ(1, m4.coords[0]);
   ASSERT_EQ(-12, m4.coords[1]);
   ASSERT_EQ(123, m4.coords[2]);

   MDCoord m5(m4);
   ASSERT_EQ(3, m5.nDim);
   ASSERT_EQ(1, m5.coords[0]);
   ASSERT_EQ(-12, m5.coords[1]);

}

TEST(MDCoord, Comparison)
{
   srand(time(NULL));
   i64 coords1[] = {rand()%10, rand()%10, rand()%10};
   i64 coords2[] = {-11,11,11};
   i64 coords3[] = {1,2};

   MDCoord m1(coords1, 3);
   MDCoord m2(coords2, 3);
   MDCoord m3(coords3, 2);

   ASSERT_TRUE(m1 == m1);
   ASSERT_FALSE(m1 == m2);
   ASSERT_FALSE(m1 == m3);
   ASSERT_TRUE(m2 != m3);
   ASSERT_FALSE(m2 != m2);
}

TEST(MDCoord, Assignment)
{
   i64 coords[] = {-1, 2, 123};
   MDCoord m1(coords, 3);
   MDCoord m2;
   m2 = m1;
   ASSERT_TRUE(m1 == m2);

   m1 += m2;
   ASSERT_EQ(3, m1.nDim);
   ASSERT_EQ(-2, m1.coords[0]);
   ASSERT_EQ(4, m1.coords[1]);
   ASSERT_EQ(246, m1.coords[2]);

   m1 -= m2;
   ASSERT_TRUE(m1 == m2);
}

TEST(MDCoord, Arithmetic)
{
   i64 coords[] = {-1, 2, 123};
   MDCoord m1(coords, 3);
   MDCoord m2;
   m2 = m1;

   MDCoord m3;
   m3 = (m1 + m2);
   ASSERT_EQ(3, m3.nDim);
   ASSERT_EQ(-2, m3.coords[0]);
   ASSERT_EQ(4, m3.coords[1]);
   ASSERT_EQ(246, m3.coords[2]);

   m3 = m1 - m2;
   ASSERT_EQ(3, m3.nDim);
   ASSERT_EQ(0, m3.coords[0]);
   ASSERT_EQ(0, m3.coords[1]);
   ASSERT_EQ(0, m3.coords[2]);
}

TEST(RowMajor, Create)
{
   srand(time(NULL));
   int n = rand() % 30 + 20;
   i64 coords[n];
   for (int i = 0; i < n; i++)
      coords[i] = i+1;
   MDCoord dim(coords, n);
   RowMajor rowMajor(dim);
   ASSERT_EQ(n, rowMajor.nDim);
   for (int i = 0; i < n; i++)
      ASSERT_EQ(i+1, rowMajor.dims[i]);

   RowMajor *rm = rowMajor.clone();
   ASSERT_EQ(n, rm->nDim);
   for (int i = 0; i < n; i++)
      ASSERT_EQ(i+1, rm->dims[i]);
   delete rm;
}

TEST(RowMajor, Linearize)
{
   u8 n = 3;
   i64 coords[n];
   Key_t size = 1;
   for (int i = 0; i < n; i++)
   {
      coords[i] = 3;
      size *= coords[i];
   }
   ASSERT_EQ(27, size);
   MDCoord dim(coords, n);
   RowMajor rm(dim);
   for (int i = 0; i < size; i++)
   {
      i64 coords[3];
      coords[0] = (i/9)%3;
      coords[1] = (i/3)%3;
      coords[2] = i%3;
      MDCoord c(coords, 3);
      ASSERT_EQ(i, rm.linearize(c));
      ASSERT_TRUE(c == rm.unlinearize((Key_t)i));
      ASSERT_TRUE(c == rm.unlinearize(rm.linearize(c)));
   }

   n = 8;
   coords[n];
   size = 1;
   for (int i = 0; i < n; i++)
   {
      coords[i] = i+3;
      size *= coords[i];
   }
   MDCoord dim2(coords, n);
   RowMajor rm2(dim2);
   for (int i = 0; i < size; i++)
      ASSERT_TRUE(i == rm2.linearize(rm2.unlinearize(i)));

}

TEST(RowMajor, Move)
{
   u8 n = 8;
   Key_t size = 1;
   i64 coords[n];
   i64 start[n];
   for (int i=0; i<n;i++)
   {
      start[i] = 0;
      coords[i]=i+3;
      size*=coords[i];
   }
   MDCoord dim(coords, n);
   MDCoord cur(start, n);
   RowMajor rm(dim);
   ASSERT_EQ(0, rm.linearize(cur));
   for (int i=1; i<size; i++)
   {
      cur = rm.move(cur, 1);
      ASSERT_EQ(i, rm.linearize(cur));
   }
   for (int i = size-1; i >0; i--)
   {
      ASSERT_EQ(i, rm.linearize(cur));
      cur = rm.move(cur,/*(KeyDiff_t)*/(-1));
   }
   ASSERT_EQ(0, rm.linearize(cur));
   cur = rm.move(cur, size-1);
   ASSERT_EQ(size-1, rm.linearize(cur));
   cur = rm.move(cur, /*(KeyDiff_t)*/(-size+1));
   ASSERT_EQ(0, rm.linearize(cur));
   int x = rand() % (size);
   cur = rm.move(cur, x);
   ASSERT_EQ(x, rm.linearize(cur));
}

TEST(RowMajor, Timing)
{
   FILE *file = fopen("timings.bin", "a");
   clock_t begin, end;
   fprintf(file, "Timing for RowMajor\n");
   fprintf(file, "Clock ticks per second: %ld\n", CLOCKS_PER_SEC);
   for (int n = 1; n < 9; n++)
   {  
      i64 coords[n];
      i64 start[n];
      Key_t size = 1;
      for (int i=0; i<n;i++)
      {
         start[i] = 0;
         coords[i]=i+3;
         size*=coords[i];
      }
      MDCoord dim(coords, n);
      ColMajor cm(dim);

      MDCoord cur1(start, n);
      begin = clock();
      for (int i=1; i<size; i++)
      {
         cur1 = cm.move(cur1, 1);
      }
      end = clock();
      fprintf(file, "ms elapsed for %d-D array using move: %ld\n", n, (end-begin)/1000);

      MDCoord cur2(start, n);
      begin = clock();
      for (int i=1; i<size; i++)
      {
         cur2 = cm.unlinearize(cm.linearize(cur2) + 1);
      }
      end = clock();
      fprintf(file, "ms elapsed for %d-D array using unlinearize/linearize: %ld\n", n, (end-begin)/1000);
   }
   fprintf(file, "\n\n");
   fclose(file);
}

TEST(RowMajor, 3D)
{
   FILE *file = fopen("timings.bin", "a");
   clock_t begin, end;
   int size = 1;
   fprintf(file, "Timing for 3-D RowMajor\n");
   fprintf(file, "Clock ticks per second: %ld\n", CLOCKS_PER_SEC);
   for (int k = 1; k < 10; k++)
   {
      i64 start[] = {0, 0, 0};
      i64 coords[] = {0, 0, 0};
      for (int i = 0; i < k; i++)
      {
         coords[0] = coords[1] = coords[2] += 10;
      }
      size = coords[0]*coords[1]*coords[2];
      RowMajor rm(MDCoord(coords, 3));

      MDCoord cur1(start, 3);
      begin = clock();
      for (int i=1; i<size; i++)
      {
         cur1 = rm.move(cur1, 1);
      }
      end = clock();
      fprintf(file, "ms elapsed for 3-D array with size %d using move: %ld\n", 10*k, (end-begin)/1000);

      MDCoord cur2(start, 3);
      begin = clock();
      for (int i=1; i<size; i++)
      {
         cur2 = rm.unlinearize(rm.linearize(cur2) + 1);
      }
      end = clock();
      fprintf(file, "ms elapsed for 3-D array with size %d  using unlinearize/linearize: %ld\n", 10*k, (end-begin)/1000);
   }
   fprintf(file, "\n\n");
   fclose(file);
}

TEST(ColMajor, Create)
{
   srand(time(NULL));
   int n = rand() % 30 + 20;
   i64 coords[n];
   for (int i = 0; i < n; i++)
      coords[i] = i+1;
   MDCoord dim(coords, n);
   ColMajor colMajor(dim);
   ASSERT_EQ(n, colMajor.nDim);
   for (int i = 0; i < n; i++)
      ASSERT_EQ(i+1, colMajor.dims[i]);

   ColMajor *cm = colMajor.clone();
   ASSERT_EQ(n, cm->nDim);
   for (int i = 0; i < n; i++)
      ASSERT_EQ(i+1, cm->dims[i]);
   delete cm;
}

TEST(ColMajor, Linearize)
{
   u8 n = 3;
   i64 coords[n];
   Key_t size = 1;
   for (int i = 0; i < n; i++)
   {
      coords[i] = 3;
      size *= coords[i];
   }
   ASSERT_EQ(27, size);
   MDCoord dim(coords, n);
   ColMajor cm(dim);
   for (int i = 0; i < size; i++)
   {
      i64 coords[3];
      coords[2] = (i/9)%3;
      coords[1] = (i/3)%3;
      coords[0] = i%3;
      MDCoord c(coords, 3);
      ASSERT_EQ(i, cm.linearize(c));
      ASSERT_TRUE(c == cm.unlinearize((Key_t)i));
      ASSERT_TRUE(c == cm.unlinearize(cm.linearize(c)));
   }

   n = 8;
   coords[n];
   size = 1;
   for (int i = 0; i < n; i++)
   {
      coords[i] = i+3;
      size *= coords[i];
   }
   MDCoord dim2(coords, n);
   ColMajor cm2(dim2);
   for (int i = 0; i < size; i++)
      ASSERT_TRUE(i == cm2.linearize(cm2.unlinearize(i)));

}

TEST(ColMajor, Move)
{
   u8 n = 8;
   Key_t size = 1;
   i64 coords[n];
   i64 start[n];
   for (int i=0; i<n;i++)
   {
      start[i] = 0;
      coords[i]=i+3;
      size*=coords[i];
   }
   MDCoord dim(coords, n);
   MDCoord cur(start, n);
   ColMajor cm(dim);
   ASSERT_EQ(0, cm.linearize(cur));
   for (int i=1; i<size; i++)
   {
      cur = cm.move(cur, 1);
      ASSERT_EQ(i, cm.linearize(cur));
   }
   for (int i = size-1; i >0; i--)
   {
      ASSERT_EQ(i, cm.linearize(cur));
      cur = cm.move(cur,(KeyDiff_t)(-1));
   }
   ASSERT_EQ(0, cm.linearize(cur));
   cur = cm.move(cur, size-1);
   ASSERT_EQ(size-1, cm.linearize(cur));
   cur = cm.move(cur, -size+1);
   ASSERT_EQ(0, cm.linearize(cur));
   int x = rand() % (size);
   cur = cm.move(cur, x);
   ASSERT_EQ(x, cm.linearize(cur));
}

TEST(ColMajor, Timing)
{
   FILE *file = fopen("timings.bin", "a");
   clock_t begin, end;
   fprintf(file, "Timing for ColMajor\n");
   fprintf(file, "Clock ticks per second: %ld\n", CLOCKS_PER_SEC);
   for (int n = 1; n < 9; n++)
   {  
      i64 coords[n];
      i64 start[n];
      Key_t size = 1;
      for (int i=0; i<n;i++)
      {
         start[i] = 0;
         coords[i]=i+3;
         size*=coords[i];
      }
      MDCoord dim(coords, n);
      ColMajor cm(dim);

      MDCoord cur1(start, n);
      begin = clock();
      for (int i=1; i<size; i++)
      {
         cur1 = cm.move(cur1, 1);
      }
      end = clock();
      fprintf(file, "ms elapsed for %d-D array using move: %ld\n", n, (end-begin)/1000);

      MDCoord cur2(start, n);
      begin = clock();
      for (int i=1; i<size; i++)
      {
         cur2 = cm.unlinearize(cm.linearize(cur2) + 1);
      }
      end = clock();
      fprintf(file, "ms elapsed for %d-D array using unlinearize/linearize: %ld\n", n, (end-begin)/1000);
   }
   fprintf(file, "\n\n");
   fclose(file);
}

TEST(ColMajor, 3D)
{
   FILE *file = fopen("timings.bin", "a");
   clock_t begin, end;
   int size = 1;
   fprintf(file, "Timing for 3-D ColMajor\n");
   fprintf(file, "Clock ticks per second: %ld\n", CLOCKS_PER_SEC);
   for (int k = 1; k < 10; k++)
   {
      i64 start[] = {0, 0, 0};
      i64 coords[] = {0, 0, 0};
      for (int i = 0; i < k; i++)
      {
         coords[0] = coords[1] = coords[2] += 10;
      }
      size = coords[0]*coords[1]*coords[2];
      ColMajor cm(MDCoord(coords, 3));

      MDCoord cur1(start, 3);
      begin = clock();
      for (int i=1; i<size; i++)
      {
         cur1 = cm.move(cur1, 1);
      }
      end = clock();
      fprintf(file, "ms elapsed for 3-D array with size %d using move: %ld\n", 10*k, (end-begin)/1000);

      MDCoord cur2(start, 3);
      begin = clock();
      for (int i=1; i<size; i++)
      {
         cur2 = cm.unlinearize(cm.linearize(cur2) + 1);
      }
      end = clock();
      fprintf(file, "ms elapsed for 3-D array with size %d  using unlinearize/linearize: %ld\n", 10*k, (end-begin)/1000);
   }
   fprintf(file, "\n\n");
   fclose(file);
}

TEST(BlockBased, Create)
{
   srand(time(NULL));
   int n = rand() % 30 + 20;
   i64 array[n];
   i64 block[n];
   u8 blockOrders[n];
   u8 microOrders[n];
   for (int i = 0; i < n; i++)
   {
      array[i] = i+4;
      block[i] = i+2;
      blockOrders[i] = i;
      microOrders[i] = n-i-1;
   }
   BlockBased bb(n, array, block, blockOrders, microOrders);
   ASSERT_EQ(n, bb.nDim);
   for (int i = 0; i < n; i++)
   {
      ASSERT_EQ(i+4, bb.arrayDims[i]);
      ASSERT_EQ(i+2, bb.blockDims[i]);
      ASSERT_EQ(i, bb.blockOrders[i]);
      ASSERT_EQ(n-i-1, bb.microOrders[i]);
   }

   BlockBased *bbptr = bb.clone();
   ASSERT_EQ(n, bb.nDim);
   for (int i = 0; i < n; i++)
   {
      ASSERT_EQ(i+4, bb.arrayDims[i]);
      ASSERT_EQ(i+2, bb.blockDims[i]);
      ASSERT_EQ(i, bb.blockOrders[i]);
      ASSERT_EQ(n-i-1, bb.microOrders[i]);
   }

   delete bbptr;
}

TEST(BlockBased, Linearize)
{
   u8 nDim = 2;
   i64 arrayDims[] = {6, 6};
   i64 blockDims[] = {3, 3};
   u8 blockOrders[] = {0, 1};
   u8 microOrders[] = {1, 0};
   Key_t size = 36;

   BlockBased bb(nDim, arrayDims, blockDims, blockOrders, microOrders);
   for (int i = 0; i < 18; i++)
   {
      i64 coords[2];
      coords[0] = (i)%3;
      coords[1] = (i/3);
      MDCoord c(coords, 2);
      ASSERT_EQ(i, bb.linearize(c));
      ASSERT_TRUE(c == bb.unlinearize((Key_t)i));
      ASSERT_TRUE(c == bb.unlinearize(bb.linearize(c)));
   }

   for (int i = 18; i < 36; i++)
   {
      i64 coords[2];
      coords[0] = (i)%3 + 3;
      coords[1] = (i)/3 - 6;
      MDCoord c(coords, 2);
      ASSERT_EQ(i, bb.linearize(c));
      ASSERT_TRUE(c == bb.unlinearize((Key_t)i));
      ASSERT_TRUE(c == bb.unlinearize(bb.linearize(c)));
   }

   for (int i = 0; i < 36; i++)
   {
      ASSERT_EQ(i, bb.linearize(bb.unlinearize(i)));
   }

   u8 n = 5;
   i64 as[n];
   i64 bs[n];
   u8 bo[n];
   u8 mo[n];
   size = 1;

   for (int i = 0; i < n; i++)
   {
      as[i] = (i+1)*(i+1);
      bs[i] = i+1;
      bo[i] = n - i -1;
      mo[i] = n - i -1;
      size *= as[i];
   }

   permute(bo, n);
   permute(mo, n);

   BlockBased bb2(n, as, bs, bo, mo);
   for (Key_t i = 0; i < size; i++)
   {
      ASSERT_EQ(i, bb2.linearize(bb2.unlinearize(i)));
   }
}

TEST(BlockBased, Move)
{
   u8 nDim = 5;
   i64 arrayDims[nDim];
   i64 blockDims[nDim];
   i64 start[nDim];
   u8 blockOrders[nDim];
   u8 microOrders[nDim];
   Key_t size = 1;

   for (int i = 0; i < nDim; i++)
   {
      arrayDims[i] = 2*(i+1)*(i+1);
      blockDims[i] = i+1;
      start[i] = 0;
      blockOrders[i] = i;
      microOrders[i] = nDim - i - 1;
      size *= arrayDims[i];
   }

   permute(blockOrders, nDim);
   permute(microOrders, nDim);

   BlockBased bb(nDim, arrayDims, blockDims, blockOrders, microOrders);
   MDCoord cur(start, nDim);

   ASSERT_EQ(0, bb.linearize(cur));
   for (int i = 1; i < size; i++)
   {
      cur = bb.move(cur, 1);
      ASSERT_EQ(i, bb.linearize(cur));
   }

   for (int i = size-1; i > 0; i--)
   {
      ASSERT_EQ(i, bb.linearize(cur));
      cur = bb.move(cur, -1);
   }

   ASSERT_EQ(0, bb.linearize(cur));
   cur = bb.move(cur, size-1);
   ASSERT_EQ(size-1, bb.linearize(cur));
   cur = bb.move(cur, -size+1);
   ASSERT_EQ(0, bb.linearize(cur));
   srand(time(NULL));
   int x = rand()%size;
   cur = bb.move(cur, x);
   ASSERT_EQ(x, bb.linearize(cur));
   
}

TEST(BlockBased, timing)
{
   FILE *file = fopen("timings.bin", "a");
   clock_t begin, end;
   fprintf(file, "Timing for 3-D BlockBased\n");
   fprintf(file, "Clock ticks per second: %ld\n", CLOCKS_PER_SEC);

   for (int n = 1; n < 5; n++)
   {
      u8 nDim = n;
      i64 arrayDims[nDim];
      i64 blockDims[nDim];
      i64 start[nDim];
      u8 blockOrders[nDim];
      u8 microOrders[nDim];
      Key_t size = 1;

      for (int i = 0; i < nDim; i++)
      {
         arrayDims[i] = (i+1)*(i+1);
         blockDims[i] = i+1;
         start[i] = 0;
         blockOrders[i] = i;
         microOrders[i] = nDim - i - 1;
         size *= arrayDims[i];
      }

      permute(blockOrders, nDim);
      permute(microOrders, nDim);

      BlockBased bb(nDim, arrayDims, blockDims, blockOrders, microOrders);

      MDCoord cur(start, nDim);
      begin = clock();
      for (int i = 1; i < size; i++)
      {
         cur = bb.move(cur, 1);
      }
      end = clock();
      fprintf(file, "ms elapsed for 3-D array with size %d using move: %ld\n", size, (end-begin)/1000);

      MDCoord cur2(start, nDim);
      begin = clock();
      for (int i=1; i<size; i++)
      {
         cur2 = bb.unlinearize(bb.linearize(cur2) + 1);
      }
      end = clock();
      fprintf(file, "ms elapsed for 3-D array with size %d  using unlinearize/linearize: %ld\n", size, (end-begin)/1000);
   }

   fprintf(file, "\n\n");
   fclose(file);
}

TEST(MDArray, Create)
{
   u8 nDim = 2;
   i64 arrayDims[] = {1000, 1000};
   StorageType type = DMA;
   Linearization *row = new RowMajor(nDim, arrayDims);
   Linearization *col = new ColMajor(nDim, arrayDims);
   i64 blockDims[] = {200, 200};
   u8 blockOrders[] = {0, 1};
   u8 microOrders[] = {0, 1};
   permute(blockOrders, nDim);
   permute(microOrders, nDim);
   Linearization *block = new BlockBased(nDim, arrayDims, blockDims,
         blockOrders, microOrders);
   const char *fileName = "test.bin";

   MDArray *array = new MDArray(nDim, arrayDims, type, row, fileName);
   MDCoord coord1 = MDCoord(2, 120, 19);
   ASSERT_TRUE(AC_OK == array->put(coord1, 12345));
   MDCoord coord2 = MDCoord(2, 500, 500);
   ASSERT_TRUE(AC_OK == array->put(coord2, 12.345));
   MDCoord coord3 = MDCoord(2, 999, 999);
   ASSERT_TRUE(AC_OK == array->put(coord3, 1000));
   Datum_t datum;
   ASSERT_TRUE(AC_OK == array->get(coord1, datum));
   ASSERT_TRUE(12345 == datum);
   ASSERT_TRUE(AC_OK == array->get(coord2, datum));
   ASSERT_TRUE(12.345 == datum);
   ASSERT_TRUE(AC_OK == array->get(coord3, datum));
   ASSERT_TRUE(1000 == datum);

/*   delete array;
   array = new MDArray(fileName);
   ASSERT_TRUE(AC_OK == array->get(coord1, datum));
   ASSERT_TRUE(12345 == datum);
   ASSERT_TRUE(AC_OK == array->get(coord2, datum));
   ASSERT_TRUE(12.345 == datum);
   ASSERT_TRUE(AC_OK == array->get(coord3, datum));
   ASSERT_TRUE(1000 == datum);*/
   MDIterator *it = array->createIterator(Dense, col);
   MDCoord coord;
   int i = 0;
   do 
   {
      it->put(i);
      i++;
   } while(it->moveNext());
   ASSERT_EQ(1000*1000, i);
   delete it;
/*   it = array->createIterator(Dense, block);
   i = 0;
   do
   {
      it->get(coord, datum);
      ASSERT_EQ(i, col->linearize(coord));
      if (coord == coord1)
         ASSERT_EQ(12345, datum);
      else if (coord == coord2)
         ASSERT_EQ(12.345, datum);
      else if (coord == coord3)
         ASSERT_EQ(1000, datum);
      else
         ASSERT_EQ(0, datum);
      i++;
   } while (it->moveNext());
   ASSERT_EQ(1000*1000, i);

   delete it;

   it = array->createNaturalIterator(Dense);
   i = 0;
   do
   {
      it->get(coord, datum);
      ASSERT_EQ(i, row->linearize(coord));
      ASSERT_EQ(i, datum);
      i++;
   } while (it->moveNext());
   ASSERT_EQ(1000*1000, i);
   delete it;*/
}
