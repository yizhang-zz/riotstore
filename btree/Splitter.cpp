#include <float.h>
#include <math.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_sf_exp.h>
#include "Splitter.h"
#include "BtreeBlock.h"
#include "BtreeDenseLeafBlock.h"
#include "BtreeSparseBlock.h"
#include <iostream>

using namespace Btree;

// Needed to force the compiler to compile the template classes
MSplitter<Datum_t> md;
MSplitter<PID_t> mp;
RSplitter<Datum_t> rd;
RSplitter<PID_t> rp;
BSplitter<Datum_t> bd(0);
BSplitter<PID_t> bp(0);
TSplitter<Datum_t> td(.6);
TSplitter<PID_t> tp(.6);
SSplitter<Datum_t> sd;
SSplitter<PID_t> sp;

template<class Value>
int Splitter<Value>::splitHelper(BlockT<Value> **orig, BlockT<Value> **newBlock,
								 PageHandle newPh, char *newImage,
								 int sp, Key_t spKey, Key_t *keys, Value *values)
								 
{
	Block::Type leftType, rightType;
	int newSize = (*orig)->sizeWithOverflow() - sp;
	int numPut;
	(*orig)->splitTypes(sp, spKey, &leftType, &rightType);
	*newBlock = static_cast<BlockT<Value>*>(Block::create(rightType, newPh,
														  newImage, spKey,
														  (*orig)->getUpperBound()));
	(*newBlock)->putRangeSorted(keys, values, newSize, &numPut);
	(*orig)->truncate(sp, spKey);
	if (leftType != (*orig)->type()) {
		BlockT<Value> *newLeft = (*orig)->switchFormat();
		delete *orig;
		*orig = newLeft;
		return 1;  // notify caller
	}
	return 0;
}

template<class Value>
int MSplitter<Value>::split(BlockT<Value> **orig, BlockT<Value> **newBlock,
							PageHandle newPh, char *newImage)
{
    /*
     * orig's dense/sparse format will not be changed for efficiency
     * reasons. The new node's default format is sparse; but if its
     * range is no larger than a dense node's capacity, then the dense
     * format is used.
     */

    int size = (*orig)->sizeWithOverflow();
    int sp = size / 2; // split before the sp-th record
	int newSize = size-sp; // size of new block
	// get the second half block
	Key_t keys[newSize];
	Value values[newSize];
	(*orig)->getRangeWithOverflow(sp, size, keys, values);

	return this->splitHelper(orig, newBlock, newPh, newImage, sp, keys[0],
							 keys, values);
}

template<class Value>
int BSplitter<Value>::split(BlockT<Value> **orig_, BlockT<Value> **newBlock,
							 PageHandle newPh, char *newImage)
{
	BlockT<Value> *&orig = *orig_;
    // start from the middle and find the closest boundary
    int left, right, sp;
    int size = orig->sizeWithOverflow();
    if (size % 2 == 0) {
        right = size / 2;
        left = right - 1;
    }
    else {
        left = right = size / 2;
    }

	Key_t keys[size];
	Value values[size];
	orig->getRangeWithOverflow(0, size, keys, values);
	
    while(true) {
        // test if can split in front of left/right position
        // loop is terminated once a split point is found
        if (keys[right-1]/boundary
			< keys[right]/boundary) { // integer comparison
            sp = right;
            break;
        }
        right++;

        if (keys[left-1]/boundary 
			< keys[left]/boundary) {
            sp = left;
            break;
        }
        left--;
    }
	Key_t spKey = (keys[sp]/boundary)*boundary;
	return this->splitHelper(orig_, newBlock, newPh, newImage, sp, spKey,
							 keys+sp, values+sp);
}

template<class Value>
int RSplitter<Value>::split(BlockT<Value> **orig_, BlockT<Value> **newBlock,
							 PageHandle newPh, char *newImage)
{
	static double capacities[] = {config->internalCapacity,
								  config->sparseLeafCapacity,
								  0,
								  config->denseLeafCapacity};
	BlockT<Value> *&orig = *orig_;
	int size = orig->sizeWithOverflow();
	Key_t lower = orig->getLowerBound();
	Key_t upper = orig->getUpperBound();
	double max = -1e9;
	Key_t keys[size];
	Value values[size];
	orig->getRangeWithOverflow(0, size, keys, values);

	int sp = 1;
	double r1, r2, smaller;
	Block::Type t1, t2;
	for (int i=1; i<size; i++) {
		// try to split before the i-th element
		orig->splitTypes(i, keys[i], &t1, &t2);
		// r = #slots / remaining domain size
		if (keys[i]-lower-i==0)
			r1 = DBL_MAX;
		else
			r1 = (capacities[t1]-i) / ((keys[i]-lower)-i);
		if (upper-keys[i]-(size-i)==0)
			r2 = DBL_MAX;
		else
			r2 = (capacities[t2]-(size-i)) / ((upper-keys[i])-(size-i));
		smaller = r1 < r2 ? r1 : r2;
		if (smaller > max) {
			max = smaller;
			sp = i;
		}
	}
	std::cout<<"split before "<<sp<<", with ratio "<<max<<std::endl;
	return this->splitHelper(orig_, newBlock, newPh, newImage, sp, keys[sp],
							 keys+sp, values+sp);
}

template<class Value>
int SSplitter<Value>::split(BlockT<Value> **orig_, BlockT<Value> **newBlock,
							PageHandle newPh, char *newImage)
{
	static double capacities[] = {config->internalCapacity,
								  config->sparseLeafCapacity,
								  0,
								  config->denseLeafCapacity};
	BlockT<Value> *&orig = *orig_;
	int size = orig->sizeWithOverflow();
	Key_t lower = orig->getLowerBound();
	Key_t upper = orig->getUpperBound();
	double max = -1e9;
	Key_t keys[size];
	Value values[size];
	orig->getRangeWithOverflow(0, size, keys, values);

	int sp = 1;
	int lcap, rcap;
	Block::Type t1, t2;
	for (int i=1; i<size; i++) {
		orig->splitTypes(i, keys[i], &t1, &t2);
		lcap = capacities[t1];
		rcap = capacities[t2];
		double x = sValue(lcap-i, rcap-(size-i), (keys[i]-lower)-i, 
						  (upper-keys[i])-(size-i));
		if (x > max) {
			max = x;
			sp = i;
		}
	}
	std::cout<<"split before "<<sp<<", with max S="<<max<<std::endl;
	return this->splitHelper(orig_, newBlock, newPh, newImage, sp, keys[sp],
							 keys+sp, values+sp);
}

template<class Value>
double SSplitter<Value>::sValue(int b1, int b2, int d1, int d2)
{
	double v1, v2;
	int d = d1+d2;
	double den = gsl_sf_lnchoose(d, d1);

	if (d1 <= b1)
		v1 = DBL_MAX;
	else {
		int stop = b1+GSL_MIN(b2, d2);
		double re = 0;
		for (int k=b1; k <= stop; k++) {
			re += k * gsl_sf_exp(gsl_sf_lnchoose(k,b1)+gsl_sf_lnchoose(d-k-1,d1-b1-1)-den);
		}
		v1 = re;
	}

	if (d2 <= b2)
		v2 = DBL_MAX;
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

template<class Value>
int TSplitter<Value>::split(BlockT<Value> **orig_, BlockT<Value> **newBlock,
							PageHandle newPh, char *newImage)
{
	BlockT<Value> *&orig = *orig_;
	int size = orig->sizeWithOverflow();
	Key_t lower = orig->getLowerBound();
	Key_t upper = orig->getUpperBound();
	Key_t keys[size];
	Value values[size];
	orig->getRangeWithOverflow(0, size, keys, values);

	// all elements in the left node should be < marker0,
	// all elements in the right node should be >= marker1
	// relative order: lower ... marker1 ... marker0 ... upper

	double r0;
	int index0, index0a, index0b;
	Key_t spKey0;
	Key_t marker0a = lower+config->sparseLeafCapacity;
	binarySearch(keys, size, marker0a, &index0a);
	double r0a = ((double) index0a)/config->sparseLeafCapacity;
	Key_t marker0b = lower+config->denseLeafCapacity;
	binarySearch(keys, size, marker0b, &index0b);
	double r0b = ((double) index0b)/config->denseLeafCapacity;
	if (r0a > r0b) {
		r0 = r0a;
		index0 = index0a;
		spKey0 = marker0a;
	}
	else {
		r0 = r0b;
		index0 = index0b;
		spKey0 = marker0b;
	}

	// try sparse and dense for the right child
	double r1;
	int index1, index1a, index1b;
	Key_t spKey1;
	Key_t marker1a = upper-config->denseLeafCapacity;
	binarySearch(keys, size, marker1a, &index1a);
	double r1a = ((double) (size-index1a))/config->denseLeafCapacity;
	Key_t marker1b = upper-config->sparseLeafCapacity;
	binarySearch(keys, size, marker1b, &index1b);
	double r1b = ((double) (size-index1b))/config->sparseLeafCapacity;
	if (r1a > r1b) {
		r1 = r1a;
		index1 = index1a;
		spKey1 = marker1a;
	}
	else {
		r1 = r1b;
		index1 = index1b;
		spKey1 = marker1b;
	}
	
	// if any child is dense enough, then pick the one that results in
	// the maximum density; otherwise split in the middle
	int sp = 0;
	key_t spKey;
	if (r0 < threshold && r1 < threshold) {
		sp = size/2;
		spKey = keys[sp];
	}
	else if (r0 > r1) {
		sp = index0;
		spKey = spKey0;
	}
	else {
		sp = index1;
		spKey = spKey1;
	}
	
	return this->splitHelper(orig_, newBlock, newPh, newImage, sp, spKey,
							 keys+sp, values+sp);
}
