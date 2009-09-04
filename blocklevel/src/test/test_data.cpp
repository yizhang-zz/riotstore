#include "../data.h"
#include "../util.h"
#include <iostream>
#include <cassert>
using namespace std;

Range r;
Range* rp = &r;
BlockHeader bh;
BlockHeader* bhp = &bh;

void testSetRange() 
{
	setRange(r, 1, 3);
	printRange(r);
	setRange(r, 0, 10);
	printRange(r);
}

void testSetBlockHeader() 
{
	setBlockHeader(bhp, BLOCK_NULL, r, 0, -10, 200);
	printBlockHeader(bh);
	setBlockHeader(bhp, BLOCK_DENSE, r, 3, 120);
	printBlockHeader(bh);
	cout<<"address of r: "<<rp<<endl;
	setBlockHeader(bhp, BLOCK_SPARSE, r, 10);
	printBlockHeader(bh);
	cout<<"address of bh: "<<bhp<<endl;
}

void testNA()
{
   Data d = R_ValueOfNA();
   assert(R_IsNA(d));
   d = 20.0;
   assert(!R_IsNA(d));
   d = 0.0;
   assert(!R_IsNA(d));
   d = -20.0;
   assert(!R_IsNA(d));
}

int main()
{
	cout<<"block size: "<<BLOCK_SIZE<<endl;
	cout<<"number of dense entries: "<<NUM_DENSE_ENTRIES<<endl;
	cout<<"size of Entry: "<<sizeof(Entry)<<endl;
	cout<<"size of Range: "<<sizeof(Range)<<endl;
	cout<<"size of BlockHeader: "<<sizeof(BlockHeader)<<endl;
	cout<<"size of SparseHeader: "<<sizeof(SparseHeader)<<endl;
	
	testSetRange();
	testSetBlockHeader();
   testNA();

	return 0;
}
