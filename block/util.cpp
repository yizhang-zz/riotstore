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
	cout<<"next block address: "<<bh.next<<endl;
	cout<<"default value: "<<bh.default_value<<endl;
	cout<<"number of entries: "<<bh.entry_count<<endl;
}

