#ifndef ROW_MAJOR_H
#define ROW_MAJOR_H

#include "Linearization.h"
#include "BlockBased.h"

template<int nDim>
class RowMajor : public BlockBased<nDim>
{
public:

	void serialize(char **p)
	{
		*((int*)(*p)) = ROW;
		*p += sizeof(int);
		memcpy(*p, this->arrayDims, sizeof(i64)*nDim);
		*p += sizeof(i64)*nDim;
	}

	RowMajor(const i64 *arrayDims) : BlockBased<nDim>(arrayDims, true)
	{
	}

    /**
     * Clone method to simulate virtual copy constructor. IMPORTANT: a
     * subclass A shoule return type A* instead of Linearization*.
     *
     * \return A pointer to a clone of subclass type.
     */
	Linearization<nDim>* clone() { return new RowMajor<nDim>(this->arrayDims); }
    
    LinearizationType getType() { return ROW; }

	std::vector<Segment> *getOverlap(const MDCoord<nDim> &begin,
			const MDCoord<nDim> &end)
	{
		using namespace std;
		vector<Segment> *v = new vector<Segment>;
		getOverlapWithBlock(MDCoord<nDim>(), this->arrayDims, this->microOrders,
				begin, end.min(this->arrayDims), v);
		return v;
		/*
		int i = nDim-1; // starting from most significant dim
		for (; i>=0 && begin[i]==0 && end[i]>=arrayDims[i]-1; --i) ;
		if (i<=0) {
			// a single contiguous segment
			v->push_back(Segment(linearize(begin),linearize(end)));
			return v;
		}
		--i;
		MDCoord<nDim> b(begin), e(end);
		for (int j=i; j>=0; --j)
			e[j] = begin[j];
		while (e[0] <= end[0]) { // most significant dim out of range means we are done
			v->push_back(Segment(b,e));
			b[i] = ++e[i]; // inc both b[i] and e[i]
			for (int j=i; j>0; --j) {
				if (e[j] > end[j]) { // carry
					b[j] = e[j] = begin[j];
					b[j-1] = ++e[j-1];
				}
			}
		}
		return v;
		*/
	}
};


#endif
