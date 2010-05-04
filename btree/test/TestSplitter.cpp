#include <gtest/gtest.h>
#include <iostream>
#include "../Btree.h"
#include "../Splitter.h"

using namespace Btree;
using namespace std;

TEST(BTree, Splitter)
{
    TSplitter rsp(.7);
	MSplitter msp;
    BTree tree("tree.bin", 100, &rsp, &msp);
	Key_t keys[] = {0,1,3,4,7,12};
	for (int i=0; i<6; i++)
	  tree.put(keys[i],keys[i]);
	tree.print();
}
