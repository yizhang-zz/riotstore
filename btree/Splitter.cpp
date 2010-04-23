#include <math.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_sf_exp.h>
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
  Key_t lower = orig->getLowerBound();
  Key_t upper = orig->getUpperBound();
  double min = std::numeric_limits<double>::max();
  int sp;
  for (int i=1; i<size-1; i++) {
	// try to split before the i-th element
	double r1, r2;
	// capacity of left child is the same as orig (node reuse)
	int rcapacity = 0;
	Block::Type t1, t2;
	orig->splitTypes(i, orig->getKey(i), t1, t2);
	rcapacity = Block::BlockCapacity[t2];
	r1 = ((orig->getKey(i)-lower)-i) / (orig->capacity-i);
	r2 = ((upper-orig->getKey(i))-(size-i)) / (rcapacity-(size-i));
	double diff = fabs(r1-r2);
	if (diff < min) {
	  min = diff;
	  sp = i;
	}
  }
  return orig->split(newPh, sp, orig->getKey(sp));
}

Block* SSplitter::split(Block *orig, PageHandle newPh)
{
  int size = orig->getSize();
  Key_t lower = orig->getLowerBound();
  Key_t upper = orig->getUpperBound();
  int sp = 0;
  double max = std::numeric_limits<double>::min();
  for (int i=1; i < size-1; i++) {
	Block::Type t1, t2;
	orig->splitTypes(i, orig->getKey(i), t1, t2);
	int lcap = Block::BlockCapacity[t1];
	int rcap = Block::BlockCapacity[t2];
	Key_t key = orig->getKey(i);
	double x = sValue(lcap-i, rcap-(size-i), (key-lower)-i, 
					  (upper-key)-(size-i));
	if (x > max) {
	  max = x;
	  sp = i;
	}
  }
  
  return orig->split(newPh, sp, orig->getKey(sp));
}

double SSplitter::sValue(int b1, int b2, int d1, int d2)
{
  double v1, v2;
  int d = d1+d2;
  double den = gsl_sf_lnchoose(d, d1);

  if (d1 <= b1)
	v1 = std::numeric_limits<double>::max();
  else {
	int stop = b1+GSL_MIN(b2, d2);
	double re = 0;
	for (int k=b1; k <= stop; k++) {
	  re += k * gsl_sf_exp(gsl_sf_lnchoose(k,b1)+gsl_sf_lnchoose(d-k-1,d1-b1-1)-den);
	}
	v1 = re;
  }

  if (d2 <= b2)
	v2 = std::numeric_limits<double>::max();
  else { 
	int stop = b2+GSL_MIN(b1, d1);
	double re = 0;
	for (int k=b2; k <= stop; k++) {
	  re += k * gsl_sf_exp(gsl_sf_lnchoose(k,b2)+gsl_sf_lnchoose(d-k-1,d2-b2-1)-den);
	}
	v2 = re;
  }
  return GSL_MIN(v1, v2);
}
