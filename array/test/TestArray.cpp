
#include <gtest/gtest.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <time.h>
#include "../RowMajor.h"
#include "../ColMajor.h"
#include "../MDCoord.h"

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
   ASSERT_EQ(123, m5.coords[2]);
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
   ASSERT_EQ(n, rowMajor.dimension->nDim);
   for (int i = 0; i < n; i++)
      ASSERT_EQ(i+1, rowMajor.dimension->coords[i]);

   RowMajor *rm = rowMajor.clone();
   ASSERT_EQ(n, rm->dimension->nDim);
   for (int i = 0; i < n; i++)
      ASSERT_EQ(i+1, rm->dimension->coords[i]);
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
      cur = rm.move(cur,(KeyDiff_t)(-1));
   }
   ASSERT_EQ(0, rm.linearize(cur));
   cur = rm.move(cur, size-1);
   ASSERT_EQ(size-1, rm.linearize(cur));
   int x = rand() % (size-1) + 1;
   cur = rm.move(cur, -1*x);
   ASSERT_EQ(size-1-x, rm.linearize(cur));
}

TEST(RowMajor, Timing)
{
   FILE *file = fopen("timings.bin", "a");
   clock_t begin, end;
   fprintf(file, "Timing for RowMajor\n");
   fprintf(file, "Clock ticks per second: %d\n", CLOCKS_PER_SEC);
   for (int n = 1; n < 10; n++)
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
      fprintf(file, "ms elapsed for %d-D array using move: %d\n", n, (end-begin)/1000);

      MDCoord cur2(start, n);
      begin = clock();
      for (int i=1; i<size; i++)
      {
         cur2 = cm.unlinearize(cm.linearize(cur2) + 1);
      }
      end = clock();
      fprintf(file, "ms elapsed for %d-D array using unlinearize/linearize: %d\n", n, (end-begin)/1000);
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
   ASSERT_EQ(n, colMajor.dimension->nDim);
   for (int i = 0; i < n; i++)
      ASSERT_EQ(i+1, colMajor.dimension->coords[i]);

   ColMajor *cm = colMajor.clone();
   ASSERT_EQ(n, cm->dimension->nDim);
   for (int i = 0; i < n; i++)
      ASSERT_EQ(i+1, cm->dimension->coords[i]);
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
   int x = rand() % (size-1) + 1;
   cur = cm.move(cur, -1*x);
   ASSERT_EQ(size-1-x, cm.linearize(cur));
}

TEST(ColMajor, Timing)
{
   FILE *file = fopen("timings.bin", "a");
   clock_t begin, end;
   fprintf(file, "Timing for ColMajor\n");
   fprintf(file, "Clock ticks per second: %d\n", CLOCKS_PER_SEC);
   for (int n = 1; n < 10; n++)
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
      fprintf(file, "ms elapsed for %d-D array using move: %d\n", n, (end-begin)/1000);

      MDCoord cur2(start, n);
      begin = clock();
      for (int i=1; i<size; i++)
      {
         cur2 = cm.unlinearize(cm.linearize(cur2) + 1);
      }
      end = clock();
      fprintf(file, "ms elapsed for %d-D array using unlinearize/linearize: %d\n", n, (end-begin)/1000);
   }
   fprintf(file, "\n\n");
   fclose(file);
}
