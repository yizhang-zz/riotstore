#include "data.h"
#include <math.h>

bool R_IsNA(double x)
{
   if (isnan(x)) 
   {
      ieee_double y;
	   y.value = x;
	   return (y.word[lw] == 1954);
   }
   return false;
}
    
void setRange(Range& range, Key lower, Key upper) 
{
	range.lowerBound = lower;
	range.upperBound = upper;
}

void setBlockHeader(BlockHeader* blockHeader, int type, Range range, unsigned nextBlock, Data def, unsigned nEntries) 
{
	blockHeader->type = type;
	blockHeader->range = range;
	blockHeader->nextBlock = nextBlock;
	blockHeader->defaultValue = def;
	blockHeader->nEntries = nEntries;
}

void setEntry(Entry& entry, Key k, Data d)
{
   entry.key = k;
   entry.data = d;
}

