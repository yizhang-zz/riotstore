#ifndef COL_MAJOR_H
#define COL_MAJOR_H

#include "Linearization.h"

template<int nDim>
class ColMajor : public BlockBased<nDim>
{
public:

	ColMajor(const i64 *arrayDims) : BlockBased<nDim>(arrayDims, false)
	{
	}

    /**
     * Clone method to simulate virtual copy constructor. IMPORTANT: a
     * subclass A shoule return type A* instead of Linearization*.
     *
     * \return A pointer to a clone of subclass type.
     */
    Linearization<nDim>* clone() { return new ColMajor<nDim>(this->arrayDims); }
    
    LinearizationType getType() { return COL; }

};


#endif
