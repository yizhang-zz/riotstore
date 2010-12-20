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
/*
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
*/
template<class Value>
int Splitter<Value>::splitHelper(BlockT<Value> **orig, BlockT<Value> **newBlock,
								 PageHandle newPh,
								 int sp, Key_t spKey, Key_t *keys, Value *values)
								 
{
	Block::Type leftType, rightType;
	int newSize = (*orig)->sizeWithOverflow() - sp;
	int numPut;
	(*orig)->splitTypes(sp, spKey, &leftType, &rightType);
	*newBlock = static_cast<BlockT<Value>*>(Block::create(rightType, newPh,
														  spKey,
														  (*orig)->getUpperBound()));
	(*newBlock)->putRangeSorted(keys, values, newSize, &numPut);
	(*orig)->truncate(sp, spKey);
#ifndef DISABLE_DENSE_LEAF
	if (leftType != (*orig)->type()) {
		BlockT<Value> *newLeft = (*orig)->switchFormat();
		delete *orig;
		*orig = newLeft;
		return 1;  // notify caller
	}
#endif
	return 0;
}

template<class Value>
int MSplitter<Value>::split(BlockT<Value> **orig, BlockT<Value> **newBlock,
							PageHandle newPh)
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

	return this->splitHelper(orig, newBlock, newPh, sp, keys[0],
							 keys, values);
}

template<class Value>
int BSplitter<Value>::split(BlockT<Value> **orig_, BlockT<Value> **newBlock,
							 PageHandle newPh)
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
	return this->splitHelper(orig_, newBlock, newPh, sp, spKey,
							 keys+sp, values+sp);
}

template<class Value>
int RSplitter<Value>::split(BlockT<Value> **orig_, BlockT<Value> **newBlock,
							 PageHandle newPh)
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
	//std::cout<<"split before "<<sp<<", with ratio "<<max<<std::endl;
	return this->splitHelper(orig_, newBlock, newPh, sp, keys[sp],
							 keys+sp, values+sp);
}

void riot_handler(const char *reason, const char *file, int line, int gsl_errno)
{
	if (gsl_errno == GSL_EUNDRFLW)
		return;
	abort();
}

template<class Value>
int SSplitter<Value>::split(BlockT<Value> **orig_, BlockT<Value> **newBlock,
							PageHandle newPh)
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
	//std::cout<<"split before "<<sp<<", with max S="<<max<<std::endl;
	return this->splitHelper(orig_, newBlock, newPh, sp, keys[sp],
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
							PageHandle newPh)
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

	Key_t spKey0 = lower+config->sparseLeafCapacity;
	int index0;
	binarySearch(keys, keys+size, spKey0, &index0);
	double r0 = ((double) index0)/config->sparseLeafCapacity;
#ifndef DISABLE_DENSE_LEAF
	Key_t spKey0_ = lower+config->denseLeafCapacity;
	int index0_;
	binarySearch(keys, keys+size, spKey0_, &index0_);
	double r0_ = ((double) index0_)/config->denseLeafCapacity;
	if (r0 < r0_) {
		r0 = r0_;
		index0 = index0_;
		spKey0 = spKey0_;
	}
#endif

	// try sparse and dense for the right child
	Key_t spKey1 = upper-config->sparseLeafCapacity;
	int index1;
	binarySearch(keys, keys+size, spKey1, &index1);
	double r1 = ((double) (size-index1))/config->sparseLeafCapacity;
#ifndef DISABLE_DENSE_LEAF
	Key_t spKey1_ = upper-config->denseLeafCapacity;
	int index1_;
	binarySearch(keys, keys+size, spKey1_, &index1_);
	double r1_ = ((double) (size-index1_))/config->denseLeafCapacity;
	if (r1 < r1_) {
		r1 = r1_;
		index1 = index1_;
		spKey1 = spKey1_;
	}
#endif
	
	// if any child is dense enough, then pick the one that results in
	// the maximum density; otherwise split in the middle
	int sp = 0;
	Key_t spKey;
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
	
	return this->splitHelper(orig_, newBlock, newPh, sp, spKey,
							 keys+sp, values+sp);
}

template class MSplitter<Datum_t>;
template class MSplitter<PID_t>;
template class RSplitter<Datum_t>;
template class RSplitter<PID_t>;
template class BSplitter<Datum_t>;
template class BSplitter<PID_t>;
template class TSplitter<Datum_t>;
template class TSplitter<PID_t>;
template class SSplitter<Datum_t>;
template class SSplitter<PID_t>;
