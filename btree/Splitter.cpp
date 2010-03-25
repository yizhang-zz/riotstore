#include <math.h>
#include "Splitter.h"
#include "BtreeBlock.h"
#include <limits>
using namespace Btree;

Block* MSplitter::split(Block *orig, PageHandle newPh)
{
    /*
     * orig's dense/sparse format will not be changed for efficiency
     * reasons. The new node's default format is sparse; but if its
     * range is no larger than a dense node's capacity, then the dense
     * format is used.
     */
    int size = orig->getSize();
    int sp = size / 2;
	return orig->split(newPh, sp, orig->getKey(sp));
    /*
    if (!orig->isLeaf())
        block = new BtreeIntBlock(newHandle, beginsAt, endsBy);
    else if (orig->isSparse())
        block = new BtreeSLeafBlock(newHandle, beginsAt, endsBy);
    else if (endsBy-beginsAt > BtreeDLeafBlock::capacity)
        block = new BtreeSLeafBlock(newHandle, beginsAt, endsBy);
    else
        block = new BtreeDLeafBlock(newHandle, beginsAt, endsBy);

    // use sparse iterator to move contents to new node
    Entry e;
    BtreeBlock::Iterator *itor = orig->getSparseIterator(beginsAt, endsBy);
    int i = 0;
    while (itor->moveNext()) {
        itor->get(e.key, e.value);
        block->put(i++, e);
    }
    delete itor;

    orig->truncate(sp);
    return block;
    */
}


Block* BSplitter::split(Block *orig, PageHandle newPh)
{
    // start from the middle and find the closest boundary
    int left, right, sp;
    int size = orig->getSize();
    if (size % 2 == 0) {
        right = size / 2;
        left = right - 1;
    }
    else {
        left = right = size / 2;
    }

    while(true) {
        // test if can split in front of left/right position
        // loop is terminated once a split point is found
        if (orig->getKey(right-1)/boundary
			< orig->getKey(right)/boundary) { // integer comparison
            sp = right;
            break;
        }
        right++;

        if (orig->getKey(left-1)/boundary 
			< orig->getKey(left)/boundary) {
            sp = left;
            break;
        }
        left--;
    }

	return orig->split(newPh, sp, orig->getKey(sp)/boundary*boundary);
}


Block* RSplitter::split(Block *orig, PageHandle newPh)
{
  int size = orig->getSize();
  double lower = orig->getLowerBound();
  double upper = orig->getUpperBound();
  double min = std::numeric_limits<double>::max();
  int sp;
  for (int i=1; i<size; i++) {
	// try to split before the i-th element
	double r1, r2;
	r1 = (orig->getKey(i)-lower-(i)) / (orig->capacity-i);
	r2 = (upper-orig->getKey(i)-(size-i)) / (orig->capacity-(size-i));
	double diff = fabs(r1-r2);
	if (diff < min) {
	  min = diff;
	  sp = i;
	}
  }
  return orig->split(newPh, sp, orig->getKey(sp));
}
