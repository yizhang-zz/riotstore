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

template<class Value>
int Splitter<Value>::splitTypes(BlockT<Value> *block, Key_t *keys, int size,
        int sp, Key_t spKey, Block::Type types[2])
{
    if (block->type() == Block::kInternal)
        types[0] = types[1] = Block::kInternal;
    else if (!this->useDenseLeaf)
        types[0] = types[1] = block->type();
    else {
        Key_t boundRange[] = {spKey - block->getLowerBound(),
            block->getUpperBound() - spKey};
        Key_t actualRange[] = {keys[sp-1] - keys[0] + 1,
            keys[size-1] - keys[sp] + 1};
        int numElements[] = {sp, size-sp};
        // Invariant: boundRange >= actualRange >= numElements
        for (int i=0; i<2; i++) {
            if (boundRange[i] <= config->denseLeafCapacity)
                types[i] = Block::kDenseLeaf;
            else if (actualRange[i] <= config->denseLeafCapacity) {
                types[i] = Block::kDenseLeaf;
                if (i == 0 && block->type() == Block::kSparseLeaf 
                        && numElements[i] <= config->sparseLeafCapacity)
                    types[i] = Block::kSparseLeaf; // favor existing format
            }
            else if (numElements[i] <= config->denseLeafCapacity) {
                if (numElements[i] <= config->sparseLeafCapacity)
                    types[i] = Block::kSparseLeaf;
                else
                    return -1;
            }
            else
                return -1;
        }
    }
    return 0;
}

template<class Value>
int Splitter<Value>::splitHelper(BlockT<Value> *orig, BlockT<Value> **newBlock,
								 PageHandle newPh, BlockPool *pool,
								 int sp, Key_t spKey, Key_t *keys, Value *values, Block::Type types[2])
								 
{
	int newSize = orig->sizeWithOverflow() - sp;
	int numPut;
	//Block::Type leftType, rightType;
	//splitTypes(*orig, sp, spKey, leftType, rightType);
	*newBlock = static_cast<BlockT<Value>*>(
            pool->create(types[1], newPh, spKey,
                orig->getUpperBound()));
	(*newBlock)->putRangeSorted(keys, values, newSize, &numPut);
	orig->truncate(sp, spKey);
	return (types[0] != orig->type());
}

template<class Value>
int MSplitter<Value>::split(BlockT<Value> *orig, BlockT<Value> **newBlock,
							PageHandle newPh, BlockPool *pool)
{
    /*
     * orig's dense/sparse format will not be changed for efficiency
     * reasons. The new node's default format is sparse; but if its
     * range is no larger than a dense node's capacity, then the dense
     * format is used.
     */

    int size = orig->sizeWithOverflow();
    int sp = size / 2; // split before the sp-th record
	//int newSize = size-sp; // size of new block
	// get the second half block
	Key_t *keys =  new Key_t[size];
	Value *values =  new Value[size];
	orig->getRangeWithOverflow(0, size, keys, values);
    Block::Type types[2];
    splitTypes(orig, keys, size, sp, keys[sp], types);
	int ret = this->splitHelper(orig, newBlock, newPh, pool, sp, keys[sp],
							 keys+sp, values+sp, types);
	delete[] keys;
	delete[] values;
	return ret;
}

template<class Value>
int BSplitter<Value>::split(BlockT<Value> *orig, BlockT<Value> **newBlock,
							PageHandle newPh, BlockPool *pool)
{
    // start from the median and find the boundary closest to the median key
    int left, right, sp;
    int size = orig->sizeWithOverflow();
	Key_t *keys = new Key_t[size];
	Value *values = new Value[size];
	orig->getRangeWithOverflow(0, size, keys, values);

    Key_t median;
    if (size % 2) {
        left = size / 2;
        right = left + 1;
        median = keys[left];
    }
    else {
        left = right = size / 2;
        median = keys[right]; // same as B splitter
    }
	
    Key_t spKey;
    Block::Type types[2];
    bool found = false;
    // try split in front of left or right
    while(!found) {
        if (keys[left-1]/boundary < keys[left]/boundary) {
            Key_t tk = (keys[left]/boundary)*boundary; // closest to median
            if (!splitTypes(orig, keys, size, left, tk, types) // valid sp
                    && (!found || abs(tk-median) < abs(spKey-median))) {
                found = true;
                sp = left;
                spKey = tk;
            }
        }
        left--;

        if (keys[right-1]/boundary < keys[right]/boundary) {
            Key_t tk = (keys[right-1]/boundary+1)*boundary; // closest to median
            if (!splitTypes(orig, keys, size, right, tk, types) // valid sp
                    && (!found || abs(tk-median) < abs(spKey-median))) {
                found = true;
                sp = right;
                spKey = tk;
            }
        }
        right++;
    }
    splitTypes(orig, keys, size, sp, spKey, types);
	int ret = this->splitHelper(orig, newBlock, newPh, pool, sp, spKey,
							 keys+sp, values+sp, types);
	delete[] keys;
	delete[] values;
	return ret;
}

template<class Value>
int RSplitter<Value>::split(BlockT<Value> *orig, BlockT<Value> **newBlock,
							PageHandle newPh, BlockPool *pool)
{
	static double capacities[] = {config->internalCapacity,
								  config->sparseLeafCapacity,
								  0,
								  config->denseLeafCapacity};
	int size = orig->sizeWithOverflow();
	Key_t lower = orig->getLowerBound();
	Key_t upper = orig->getUpperBound();
	double max = -1e9;
	Key_t *keys =  new Key_t[size];
	Value *values =  new Value[size];
	orig->getRangeWithOverflow(0, size, keys, values);

	int sp = 1;
	double r1, r2, smaller;
	Block::Type types[2];
	for (int i=1; i<size; i++) {
		// try to split before the i-th element
		if (splitTypes(orig, keys, size, i, keys[i], types) != 0)
            continue;
		// r = #slots / remaining domain size
		if (keys[i]-lower-i==0)
			r1 = DBL_MAX;
		else
			r1 = (capacities[types[0]]-i) / ((keys[i]-lower)-i);
		if (upper-keys[i]-(size-i)==0)
			r2 = DBL_MAX;
		else
			r2 = (capacities[types[1]]-(size-i)) / ((upper-keys[i])-(size-i));
		smaller = r1 < r2 ? r1 : r2;
		if (smaller > max) {
			max = smaller;
			sp = i;
		}
	}
	//std::cout<<"split before "<<sp<<", with ratio "<<max<<std::endl;
    splitTypes(orig, keys, size, sp, keys[sp], types);
	int ret = this->splitHelper(orig, newBlock, newPh, pool, sp, keys[sp],
							 keys+sp, values+sp, types);
	delete[] keys;
	delete[] values;
	return ret;
}

void riot_handler(const char *reason, const char *file, int line, int gsl_errno)
{
	if (gsl_errno == GSL_EUNDRFLW)
		return;
	abort();
}

template<class Value>
int SSplitter<Value>::split(BlockT<Value> *orig, BlockT<Value> **newBlock,
							PageHandle newPh, BlockPool *pool)
{
	static double capacities[] = {config->internalCapacity,
								  config->sparseLeafCapacity,
								  0,
								  config->denseLeafCapacity};
	int size = orig->sizeWithOverflow();
	Key_t lower = orig->getLowerBound();
	Key_t upper = orig->getUpperBound();
	double max = -1e9;
	Key_t *keys = new Key_t[size];
	Value *values = new Value[size];
	orig->getRangeWithOverflow(0, size, keys, values);

	int sp = 1;
	int lcap, rcap;
	Block::Type types[2];
	for (int i=1; i<size; i++) {
		if (splitTypes(orig, keys, size, i, keys[i], types) != 0)
            continue;
		lcap = capacities[types[0]];
		rcap = capacities[types[1]];
		double x = sValue(lcap-i, rcap-(size-i), (keys[i]-lower)-i, 
						  (upper-keys[i])-(size-i));
		if (x > max) {
			max = x;
			sp = i;
		}
	}
	//std::cout<<"split before "<<sp<<", with max S="<<max<<std::endl;
    splitTypes(orig, keys, size, sp, keys[sp], types); 
	int ret = this->splitHelper(orig, newBlock, newPh, pool, sp, keys[sp],
							 keys+sp, values+sp, types);
	delete[] keys;
	delete[] values;
	return ret;
}

template<class Value>
double SSplitter<Value>::sValue(int b1, int b2, int d1, int d2)
{
	double v1, v2;
	int d = d1+d2;
	double den = gsl_sf_lnchoose(d, d1);

	if (d1 < b1 && d2 < b2)
		return DBL_MAX;
    if (d1 < b1)
        v1 = 0;
	else {
		int stop = b1+GSL_MIN(b2, d2);
		double re = 0;
		for (int k=b1; k <= stop; k++) {
			re += k * gsl_sf_exp(gsl_sf_lnchoose(k,b1)+gsl_sf_lnchoose(d-k-1,d1-b1-1)-den);
		}
		v1 = re;
	}

	if (d2 < b2)
		v2 = 0;
	else { 
		int stop = b2+GSL_MIN(b1, d1);
		double re = 0;
		for (int k=b2; k <= stop; k++) {
			re += k * gsl_sf_exp(gsl_sf_lnchoose(k,b2)+gsl_sf_lnchoose(d-k-1,d2-b2-1)-den);
		}
		v2 = re;
	}
	return v1+v2;
}

template<class Value>
int TSplitter<Value>::split(BlockT<Value> *orig, BlockT<Value> **newBlock,
							PageHandle newPh, BlockPool *pool)
{
	int size = orig->sizeWithOverflow();
	Key_t lower = orig->getLowerBound();
	Key_t upper = orig->getUpperBound();
	Key_t *keys =  new Key_t[size];
	Value *values =  new Value[size];
	orig->getRangeWithOverflow(0, size, keys, values);

	// all elements in the left node should be < marker0,
	// all elements in the right node should be >= marker1
	// relative order: lower ... marker1 ... marker0 ... upper

	Key_t spKey0 = lower+config->sparseLeafCapacity;
	int index0;
	binarySearch(keys, keys+size, spKey0, &index0);
	double r0 = ((double) index0)/config->sparseLeafCapacity;
    if (this->useDenseLeaf) {
        Key_t spKey0_ = lower+config->denseLeafCapacity;
        int index0_;
        binarySearch(keys, keys+size, spKey0_, &index0_);
        double r0_ = ((double) index0_)/config->denseLeafCapacity;
        if (r0 < r0_) {
            r0 = r0_;
            index0 = index0_;
            spKey0 = spKey0_;
        }
    }

	// try sparse and dense for the right child
	Key_t spKey1 = upper-config->sparseLeafCapacity;
	int index1;
	binarySearch(keys, keys+size, spKey1, &index1);
	double r1 = ((double) (size-index1))/config->sparseLeafCapacity;
    if (this->useDenseLeaf) {
        Key_t spKey1_ = upper-config->denseLeafCapacity;
        int index1_;
        binarySearch(keys, keys+size, spKey1_, &index1_);
        double r1_ = ((double) (size-index1_))/config->denseLeafCapacity;
        if (r1 < r1_) {
            r1 = r1_;
            index1 = index1_;
            spKey1 = spKey1_;
        }
    }
	
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
	
    Block::Type types[2];
    splitTypes(orig, keys, size, sp, spKey, types);
	int ret = this->splitHelper(orig, newBlock, newPh, pool, sp, spKey,
							 keys+sp, values+sp, types);
	delete[] keys;
	delete[] values;
	return ret;
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
