#ifndef ROW_MAJOR_H
#define ROW_MAJOR_H

#include "Linearization.h"
#include "BlockBased.h"

template<int nDim>
class RowMajor : public BlockBased<nDim>
{
public:

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

};


#endif
