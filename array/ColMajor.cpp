
#include <assert.h>
#include <math.h>
#include "ColMajor.h"


ColMajor::ColMajor(const MDCoord &coord)
{
   assert(coord.nDim != 0);
   for (int k = 0; k < coord.nDim; k++)
      assert(coord.coords[k] != 0); // cannot have dimension length of 0
   dimension = new MDCoord(coord);
}

ColMajor::~ColMajor()
{
   delete dimension;
}

Key_t ColMajor::linearize(const MDCoord &coord)
{
   assert(coord.nDim == dimension->nDim);
   assert(coord.coords[dimension->nDim - 1] < dimension->coords[dimension->nDim
         - 1]);
   Key_t key = coord.coords[dimension->nDim - 1];
   for (int k = dimension->nDim - 2; k >= 0; k--)
   {
      assert(coord.coords[k] < dimension->coords[k]);
      key = key*dimension->coords[k] + coord.coords[k];
      // key *= dimension.coords[k];
      // key += coord[k];
   }
   return key;
}

MDCoord ColMajor::unlinearize(Key_t key)
{
   i64 coords[dimension->nDim];
   coords[0] = key % dimension->coords[0];
   for (int k = 1; k < dimension->nDim; k++)
   {
      key /= dimension->coords[k-1];
      coords[k] = key % dimension->coords[k];
   }
   return MDCoord(coords, dimension->nDim);
}

MDCoord ColMajor::move(const MDCoord &from, KeyDiff_t diff)
{
   assert(from.nDim == dimension->nDim);
   MDCoord to(from);
   to.coords[0] += diff;
   for (int k = 0; k < dimension->nDim; k++)
   {
      if (to.coords[k] < dimension->coords[k]) // carry-over done propogating
         break;
      if (k < dimension->nDim - 1)
         to.coords[k+1] += to.coords[k]/dimension->coords[k];
      to.coords[k] %= dimension->coords[k];
   }
   return to;
}

ColMajor* ColMajor::clone()
{
   return new ColMajor(*(this->dimension));
}

