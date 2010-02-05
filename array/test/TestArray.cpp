
#include <gtest/gtest.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include "../RowMajor.h"
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
}

TEST(RowMajor, Linearize)
{
   srand(time(NULL));
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
      ASSERT_TRUE(c == rm.unlinearize(rm.linearize(c)));
   }

   srand(time(NULL));
   n = rand() % 10 + 1;
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
   srand(time(NULL));
   u8 n = rand() % 10 + 1;
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
   for (int i = size-1; i >0; i++)
   {
      ASSERT_EQ(i, rm.linearize(cur));
      cur = rm.move(cur,-1);
   }
   ASSERT_EQ(0, rm.linearize(cur));
   cur = rm.move(cur, size);
   ASSERT_EQ(size-1, rm.linearize(cur));
   int x = rand() % (size-1) + 1;
   cur = rm.move(cur, -1*x);
   ASSERT_EQ(size-1-x, rm.linearize(cur));
}
