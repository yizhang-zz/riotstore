#include "Linearization.h"
#include "BlockBased.h"
#include "RowMajor.h"
#include "ColMajor.h"

template<int nDim>
Linearization<nDim> * Linearization<nDim>::parse(char **p)
{
	Linearization<nDim> *l = NULL;
	int *t = (int*) *p;
	*p += sizeof(int);
	switch(*t) {
	case BLOCK:
		l = new BlockBased<nDim>(
								 (i64*)(*p),
								 (i64*)(*p+sizeof(i64)*nDim),
								 (u8*)(*p+sizeof(i64)*nDim*2),
								 (u8*)(*p+sizeof(i64)*nDim*2+sizeof(u8)*nDim));
		*p += sizeof(u8)*nDim;
		break;
	case ROW:
		l = new RowMajor<nDim>((i64*)*p);
		*p += sizeof(i64)*nDim;
		break;
	case COL:
		l = new ColMajor<nDim>((i64*)*p);
		*p += sizeof(i64)*nDim;
		break;
	default:
		Error("Unknown linearization type");
	}
	return l;
}

template class Linearization<2>;
