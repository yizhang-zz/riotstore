#ifndef COL_MAJOR_H
#define COL_MAJOR_H

#include "Linearization.h"

template<int nDim>
class ColMajor : public BlockBased<nDim>
{
public:

	void serialize(char **p)
	{
		*((int*)(*p)) = COL;
		*p += sizeof(int);
		memcpy(*p, this->arrayDims, sizeof(i64)*nDim);
		*p += sizeof(i64)*nDim;
	}

	ColMajor(const i64 *arrayDims) : BlockBased<nDim>(arrayDims, false)
	{
	}

    /**
     * Clone method to simulate virtual copy constructor. IMPORTANT: a
     * subclass A shoule return type A* instead of Linearization*.
     *
     * \return A pointer to a clone of subclass type.
     */
    Linearization<nDim>* clone() const { return new ColMajor<nDim>(this->arrayDims); }
    
    LinearizationType getType() const { return COL; }

	std::vector<Segment> *getOverlap(const MDCoord<nDim> &begin,
			const MDCoord<nDim> &end)
	{
		using namespace std;
		vector<Segment> *v = new vector<Segment>;
		getOverlapWithBlock(MDCoord<nDim>(), this->arrayDims, this->microOrders,
				begin, end.min(this->arrayDims), v);
		return v;
		/*
		int i = 0; // starting from least significant dim
		for (; i<nDim && begin[i]==0 && end[i]>=arrayDims[i]-1; ++i) ;
		if (i>=nDim-1) {
			// a single contiguous segment
			v->push_back(Segment(linearize(begin),linearize(end)));
			return v;
		}
		++i;
		MDCoord<nDim> b(begin), e(end);
		for (int j=i; j<nDim; ++j)
			e[j] = begin[j];
		while (e[nDim-1] <= end[nDim-1]) {
			v->push_back(Segment(b,e));
			b[i] = ++e[i]; // inc both b[i] and e[i]
			for (int j=i; j<nDim-1; ++j) {
				if (e[j] > end[j]) { // carry
					b[j] = e[j] = begin[j];
					b[j+1] = ++e[j+1];
				}
			}
		}
		return v;
		*/
	}
};


#endif
