#include <assert.h>
#include <math.h>
#include <string.h>
#include "ColMajor.h"


ColMajor::ColMajor(const MDCoord &coord)
{
   assert(coord.nDim != 0);
   for (int k = 0; k < coord.nDim; k++)
      assert(coord.coords[k] > 0); // cannot have dimension length of 0

   this->dim = coord;
}

ColMajor::~ColMajor()
{
}

Key_t ColMajor::linearize(const MDCoord &coord)
{
    u8 nDim = dim.nDim;
    i64 *dims = dim.coords;
   assert(coord.nDim == nDim);
   Key_t key = 0;
   for (int k = nDim - 1; k >= 0; k--)
   {
      assert(0 <= coord.coords[k] && coord.coords[k] < dims[k]);
      key = key*dims[k] + coord.coords[k];
   }
   return key;
}

MDCoord ColMajor::unlinearize(Key_t key)
{
    u8 nDim = dim.nDim;
    i64 *dims = dim.coords;

   i64 coords[nDim];
   for (int k = 0; k < nDim; k++)
   {
      coords[k] = key % dims[k];
      key /= dims[k];
   }
   return MDCoord(coords, nDim);
}

MDCoord ColMajor::move(const MDCoord &from, KeyDiff_t diff)
{
    u8 nDim = dim.nDim;
    i64 *dims = dim.coords;

   assert(from.nDim == nDim);
   for (int i = 0; i < nDim; i++)
      assert(0 <= from.coords[i] && from.coords[i] < dims[i]);

   MDCoord to(from);
   to.coords[0] += diff;
   for (int k = 0; k < nDim-1; k++)
   {
      if (0 <= to.coords[k] && to.coords[k] < dims[k]) // carry-over done propogating
         break;

      to.coords[k+1] += to.coords[k]/dims[k];
      if ((to.coords[k] %= dims[k]) < 0)
      {
         to.coords[k] += dims[k];
         to.coords[k+1]--;
      }
   }

   // most significant bit
   if (to.coords[nDim-1] < 0 || to.coords[nDim-1] >= dims[nDim-1])
   {
      if ((to.coords[nDim-1] %= dims[nDim-1]) < 0)
         to.coords[nDim-1] += dims[nDim-1];
   }
   return to;
}

ColMajor* ColMajor::clone()
{
   return new ColMajor(dim);
}

LinearizationType ColMajor::getType()
{
   return COL;
}

bool ColMajor::equals(Linearization *l)
{
    if (typeid(*l)!=typeid(*this))
        return false;
    ColMajor *ll = (ColMajor*) l;
    return ll->dim == this->dim;
}
