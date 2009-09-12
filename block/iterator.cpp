#include "iterator.h"

/*
current Iterator implementation for DenseBlock only
*/

DenseBlockIterator::DenseBlockIterator(Range range, Datum* initial) 
{
	/*
	set key and current to correspond to Datum before initial, so
	after first hasNext() call the values correspond to initial Datum
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

Datum DenseBlockIterator::getDatum() 
{
	return *current;
}

EntryIterator::EntryIterator(unsigned numEntries, Entry* initial)
{
   current = 0; // inherited Datum* unused
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

Datum EntryIterator::getDatum()
{
   return cur->datum;
}

Entry EntryIterator::getEntry()
{
   return *cur;
}
