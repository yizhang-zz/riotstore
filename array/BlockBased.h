#ifndef BLOCK_BASED_H
#define BLOCK_BASED_H

#include "Linearization.h"

template<int nDim>
class BlockBased : public Linearization<nDim>
{
public:

	void serialize(char **p)
	{
		*((int*)(*p)) = BLOCK;
		*p += sizeof(int);
		memcpy(*p, this->arrayDims, sizeof(i64)*nDim);
		*p += sizeof(i64)*nDim;
		memcpy(*p, blockDims, sizeof(i64)*nDim);
		*p += sizeof(i64)*nDim;
		memcpy(*p, blockOrders, sizeof(u8)*nDim);
		*p += sizeof(u8)*nDim;
		memcpy(*p, microOrders, sizeof(u8)*nDim);
		*p += sizeof(u8)*nDim;
	}

    void setDims(const MDCoord<nDim> &arrayDims)
    {
		memcpy(this->arrayDims, &arrayDims[0], nDim*sizeof(i64));
		blockSize = 1;
		for (int i = 0; i < nDim; i++) {
            if (blockDims[i] > arrayDims[i])
                blockDims[i] = arrayDims[i];
			blockSize *= blockDims[i];
			numBlocks[i] = (1+(arrayDims[i]-1)/blockDims[i]);
			this->actualDims[i] = blockDims[i]*numBlocks[i];
		}
    }

	BlockBased(const i64 *arrayDims, const i64 *blockDims, 
			const u8 *blockOrders, const u8 *microOrders)
	{
		memcpy(this->blockDims, blockDims, nDim*sizeof(i64));
		memcpy(this->blockOrders, blockOrders, nDim*sizeof(u8));
		memcpy(this->microOrders, microOrders, nDim*sizeof(u8));
		BlockBased<nDim>::setDims(MDCoord<nDim>(arrayDims));
	}

	/* Constructs a BlockBased linearization where the entire array
	   consists of only one block. 2nd parameter tells if the traversal
	   order within the block is row-major or column-major in the
	   general sense.
	 */
	BlockBased(const i64 *arrayDims_, bool mostSignificantFirst)
	{
		memcpy(this->blockDims, arrayDims_, nDim*sizeof(i64));
		if (mostSignificantFirst) {
			for (int i=0; i<nDim; ++i) {
				this->blockOrders[i] = i;
				this->microOrders[i] = i;
			}
		}
		else {
			for (int i=0; i<nDim; ++i) {
				this->blockOrders[i] = nDim-1-i;
				this->microOrders[i] = nDim-1-i;
			}
		}
		BlockBased<nDim>::setDims(MDCoord<nDim>(arrayDims_));
	}

    /**
     * Linearizes the given coord.
     *
     * Given \f$ N_1\times N_2\times \cdots \times N_m \f$ matrix,
     * linearization of given tuple \f$ \( n_1, n_2, \ldots , n_m) \f$ is given
     * by
     * \f$ f(n_1, n_2, \ldots , n_m) = \sum_{i=1}^{m}(\prod_{k=i+1}^{m}N_k\)n_i\
     * 
     * \param coord The coord to be linearized.
     * \return Result of linearization.
     */
    Key_t linearize(const MDCoord<nDim> &coord) const
	{
		if (nDim == 1) {
			return (Key_t)coord[0];
		}
		for (int i=0; i<nDim; ++i)
			if (coord[i] >= this->actualDims[i])
				return MAX_KEY; // makes it invalid

		i64 blockCoords[nDim];
		i64 microCoords[nDim];
		for (int i = 0; i < nDim; i++)
		{
			blockCoords[i] = coord[i] / blockDims[i];
			microCoords[i] = coord[i] % blockDims[i];
		}

		// from most significant dim to least significant dim
		i64 blockIndex = blockCoords[blockOrders[0]];
		i64 microIndex = microCoords[microOrders[0]];
		for (int i=1; i<nDim; ++i) {
			blockIndex *= numBlocks[blockOrders[i]];
			blockIndex += blockCoords[blockOrders[i]];
			microIndex *= blockDims[microOrders[i]];
			microIndex += microCoords[microOrders[i]];
		}
		return (Key_t)(blockSize*blockIndex+microIndex);
	}

    /**
     * Unlinearizes the given 1-D coord.
     *
     * \param key Key in 1-D to be unlinearized.
     * \return The result f unlinearization.
     */
    MDCoord<nDim> unlinearize(Key_t key) const
	{
		if (nDim == 1)
			return MDCoord<nDim>(key);
		i64 k = (i64)key;
		i64 blockIndex = k / blockSize;
		i64 microIndex = k % blockSize;
		i64 blockCoords[nDim];
		i64 microCoords[nDim];
		for (int i=nDim-1; i>=0; --i) {
			u8 j = blockOrders[i];
			blockCoords[j] = blockIndex % numBlocks[j];
			blockIndex /= numBlocks[j];
			microCoords[j] = microIndex % blockDims[j];
			microIndex /= blockDims[j];
		}
		MDCoord<nDim> ret;
		for (int i=0; i<nDim; ++i) {
			ret[i] = blockCoords[i]*blockDims[i]+microCoords[i];
		}
		return ret;
	}

    /*
     * Incrementally linearizes the coord specified as the sum of the
     * remembered last state and the give difference. Provided as a
     * potential optimization for quick linearization if the change in
     * coordinates is incremental.
     *
     * \param diff The difference in coordinate based on last state.
     * \return The result of linearization.
     */
    // virtual Key_t linearizeIncremental(MDCoord<nDim> &diff) = 0;

    /**
     * Incrementally calculate \f$y\f$ such that \f$ f(y)=f(from)+diff
     * \f$ from the given anchor and diff. Provided as a potential
     * optimization for quick calculation of the next or pervious
     * coordinate when iterating through an array.
     *
     * \param from The anchor coordinate in n-D space.
     * \param diff The difference in linearized space.
     * \return The new coordinate in n-D space.
     */
    MDCoord<nDim> move(const MDCoord<nDim> &from, KeyDiff_t diff) const
	{
		using namespace std;
		i64 blockCoords[nDim];
		i64 microCoords[nDim];
		for (int i = 0; i < nDim; i++)
		{
			blockCoords[i] = from[i] / blockDims[i];
			microCoords[i] = from[i] % blockDims[i];
		}
		i64 blockDiff = diff / blockSize;
		i64 microDiff = diff % blockSize;
		// move to the right position inside the block
		u8 dim;
		i64 carry = 0;
		for (int i=nDim-1; i>=0; --i) {
			dim = this->microOrders[i];
			microCoords[dim] += carry + microDiff % this->blockDims[dim];
			if (microCoords[dim] < 0) {
				microCoords[dim] += this->blockDims[dim];
				carry = -1;
				//microCoords[this->microOrders[i-1]]--;
			}
			else if (microCoords[dim] >= this->blockDims[dim]) {
				microCoords[dim] -= this->blockDims[dim];
				carry = 1;
				//microCoords[this->microOrders[i-1]]++;
			}
			else
				carry = 0;
			microDiff /= this->blockDims[dim];
		}

		// move to the right block
		for (int i=nDim-1; i>=0; --i) {
			dim = this->blockOrders[i];
			blockCoords[dim] += carry + blockDiff % this->numBlocks[dim];
			if (blockCoords[dim] < 0) {
				blockCoords[dim] += this->numBlocks[dim];
				carry = -1;
			}
			else if (blockCoords[dim] >= this->numBlocks[dim]) {
				blockCoords[dim] -= this->numBlocks[dim];
				carry = 1;
			}
			else
				carry = 0;
			blockDiff /= this->numBlocks[dim];
		}
		
		MDCoord<nDim> ret;
		for (int i=0; i<nDim; ++i) {
			ret[i] = blockCoords[i]*blockDims[i]+microCoords[i];
		}
		return ret;
	}

    /**
     * Clone method to simulate virtual copy constructor. IMPORTANT: a
     * subclass A shoule return type A* instead of Linearization*.
     *
     * \return A pointer to a clone of subclass type.
     */
    Linearization<nDim>* clone() const
	{
		return new BlockBased<nDim>(this->arrayDims, blockDims, blockOrders, microOrders);
	}

    bool equals(Linearization<nDim> *l) const 
	{
		if (l->getType() != this->getType())
			return false;
		BlockBased<nDim> *ll = static_cast<BlockBased<nDim>*>(l);
		return (equals(this->arrayDims,ll->arrayDims)
				&& equals(blockDims,ll->blockDims)
				&& equals(blockOrders, ll->blockOrders)
				&& this->equals(microOrders, ll->microOrders));
	}

    LinearizationType getType() const { return BLOCK; }

    Linearization<nDim> *transpose() const
	{
		i64 ad[nDim];
		for (int i=0; i<nDim; i++)
			ad[i] = this->arrayDims[nDim-1-i];
		i64 bd[nDim];
		for (int i=0; i<nDim; i++)
			bd[i] = blockDims[nDim-1-i];
		u8 bo[nDim];
		for (int i=0; i<nDim; i++)
			bo[i] = blockOrders[nDim-1-i];
		u8 mo[nDim];
		for (int i=0; i<nDim; i++)
			mo[i] = microOrders[nDim-1-i];
		return new BlockBased<nDim>(ad, bd, bo, mo);
	}

	MDCoord<nDim> getBlockDims() const
	{
		return MDCoord<nDim>(blockDims);
	}

	i64 getBlockSize() const { return blockSize; }


	void getOverlapWithBlock(const MDCoord<nDim> &b, const MDCoord<nDim> &e,
			u8 *orders, const MDCoord<nDim> &begin,
			const MDCoord<nDim> &end, std::vector<Segment> *v)
	{
		int i = nDim-1; // starting from least significant dim
		for (; i>=0 && begin[orders[i]]==b[orders[i]] 
				&& end[orders[i]]==e[orders[i]]; --i)
			;
		if (i<=0) {
			// a single contiguous segment
			v->push_back(Segment(linearize(begin),linearize(end)));
			return;
		}
		--i;
		MDCoord<nDim> b_(begin), e_(end);
		for (int j=i; j>=0; --j)
			e_[orders[j]] = begin[orders[j]];
		// most significant dim overflow means we are done
		while (e_[orders[0]] <= end[orders[0]]) { 
			v->push_back(Segment(linearize(b_), linearize(e_)));
			b_[orders[i]] = ++e_[orders[i]]; // inc both
			for (int j=i; j>0; --j) {
				u8 k = orders[j];
				if (e_[k] > end[k]) { // carry
					b_[k] = e_[k] = begin[k];
					b_[orders[j-1]] = ++e_[orders[j-1]];
				}
				else
					break;
			}
		}
	}

	std::vector<Segment> *getOverlap(const MDCoord<nDim> &begin,
			const MDCoord<nDim> &end)
	{
		std::vector<Segment> *v = new std::vector<Segment>;
		// coords in begin and end are inclusive
		MDCoord<nDim> begin_, end_;
		for (int i=0; i<nDim; ++i) {
			u8 k = blockOrders[i];
			begin_[k] = begin[k] / blockDims[k] * blockDims[k];
			end_[k] = (end[k] / blockDims[k] + 1) * blockDims[k] - 1; // inclusive
		}
		// iterate through the blocks and find their intersections with
		// the given rectangle
		MDCoord<nDim> b(begin_), e(begin_+blockDims-MDCoord<nDim>(1));
		// b and e define the first block
		while (e[blockOrders[0]] <= end_[blockOrders[0]]) {
			// find intersection
			getOverlapWithBlock(b,e,microOrders,begin.max(b),end.min(e),v);
			u8 k = blockOrders[nDim-1];
			u8 l;
			b[k] += blockDims[k];
			e[k] += blockDims[k];
			for (int i=nDim-1; i>0; --i) {
				k = blockOrders[i];
				if (e[k] > end_[k]) { // carry
					l = blockOrders[i-1];
					b[l] += blockDims[l];
					e[l] += blockDims[l];
					b[k] = begin_[k];
					e[k] = b[k] + blockDims[k] - 1;
				}
				else
					break;
			}
		}
		return v;
	}

	/*
    void getBlock(Key_t key, Key_t &begin, Key_t &end)
	{
		i64 blockCoords[nDim];
		i64 microCoords[nDim];
		u8 k = blockOrders[0];
		u8 l = blockOrders[1];
		blockCoords[k] = key/arrayDims[l]/blockDims[k];
		Key_t temp = key - blockCoords[k]*blockDims[k]*arrayDims[l];
		if (remainders[k] != 0 && blocksPerArray[k]==blockCoords[k]) {
			blockCoords[l] = temp/blockDims[l]/remainders[k];
			temp -= blockDims[l]*remainders[k]*blockCoords[l];
		}
		else {
			blockCoords[l] = temp/blockDims[l]/blockDims[k];
			temp -= blockDims[l]*blockDims[k]*blockCoords[l];
		}
		begin = linearize(MDCoord(2, blockCoords[0]*blockDims[0],
					blockCoords[1]*blockDims[1]));
		if (blockCoords[l]==blocksPerArray[l]) {
			blockCoords[l] = 0;
			blockCoords[k]++;
			if (blockCoords[k]>blocksPerArray[k]) { // end of entire array
				end = arrayDims[0]*arrayDims[1]-1;
				return;
			}
		}
		else {
			blockCoords[l]++;
		}
		end = linearize(MDCoord(2, blockCoords[0]*blockDims[0],
					blockCoords[1]*blockDims[1]))-1;
	}
 
	void getBlock(Key_t key, MDCoord<nDim> &begin, MDCoord<nDim> &end)
	{
		i64 blockCoords[nDim];
		i64 microCoords[nDim];
		u8 k = blockOrders[0];
		u8 l = blockOrders[1];
		blockCoords[k] = key/arrayDims[l]/blockDims[k];
		Key_t temp = key - blockCoords[k]*blockDims[k]*arrayDims[l];
		if (remainders[k] != 0 && blocksPerArray[k]==blockCoords[k]) {
			blockCoords[l] = temp/blockDims[l]/remainders[k];
			temp -= blockDims[l]*remainders[k]*blockCoords[l];
		}
		else {
			blockCoords[l] = temp/blockDims[l]/blockDims[k];
			temp -= blockDims[l]*blockDims[k]*blockCoords[l];
		}
		microOrders[0] = microOrders[1] = 0; // begining of the block
		begin = (MDCoord(2, blockCoords[0]*blockDims[0],
					blockCoords[1]*blockDims[1]));
		if (blockCoords[l]==blocksPerArray[l]) {
			blockCoords[l] = 0;
			blockCoords[k]++;
		}
		else {
			blockCoords[l]++;
		}
		end = (MDCoord(2, blockCoords[0]*blockDims[0],
					blockCoords[1]*blockDims[1]));
	}
    */
	i64 numBlocks[nDim];
	i64 blockDims[nDim]; /// size of block in each dimension
	u8 blockOrders[nDim]; /// ordering of dimensions between blocks, descending order of significance
	u8 microOrders[nDim]; /// ordering of elements inside a block, descending order of significance
protected:
    i64 blockSize;

	template<class T>
    bool equals(const T *a, const T *b) const 
	{
        for (int i=0; i<nDim; i++)
            if (a[i]!=b[i])
                return false;
        return true;
    }
};


#endif
