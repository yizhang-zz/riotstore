#include "iterator.h"

/*
current Iterator implementation for DenseBlock only
*/

DenseBlockIterator::DenseBlockIterator(Range range, Data* initial) 
{
	/*
	set key and current to correspond to Data before initial, so
	after first hasNext() call the values correspond to initial Data
	*/

	key = range.lowerBound - 1;
	current = initial - 1;
	end = initial + range.upperBound - range.lowerBound;
}

bool DenseBlockIterator::hasNext() 
{
	key++;
	return (current++)!=end;
}

Key DenseBlockIterator::getKey() 
{
	return key; 
}

Data DenseBlockIterator::getData() 
{
	return *current;
}

EntryIterator::EntryIterator(unsigned numEntries, Entry* initial)
{
   current = 0; // inherited Data* unused
   cur = initial - 1;
   end = initial + numEntries - 1;
}

bool EntryIterator::hasNext()
{
   return (cur++)!=end;
}

Key EntryIterator::getKey()
{
   return cur->key;
}

Data EntryIterator::getData()
{
   return cur->data;
}

Entry EntryIterator::getEntry()
{
   return *cur;
}
