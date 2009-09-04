#include "util.h"
#include <iostream>
using namespace std;

void printRange(Range& r) 
{
	cout<<"range: ("<<r.lowerBound<<", "<<r.upperBound<<")"<<endl;
}

void printBlockHeader(BlockHeader& bh) 
{
	cout<<"type: "<<bh.type<<endl;
	printRange(bh.range);
	cout<<"next block address: "<<bh.nextBlock<<endl;
	cout<<"default value: "<<bh.defaultValue<<endl;
	cout<<"number of entries: "<<bh.nEntries<<endl;
}

