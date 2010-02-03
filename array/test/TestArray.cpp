
#include <gtest/gtest.h>
#include <stdlib.h>
#include <time.h>
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
   MDCoord m3(3, 1, 12, 123);
   ASSERT_EQ(3, m3.nDim);
   ASSERT_EQ(1, m3.coords[0]);
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
