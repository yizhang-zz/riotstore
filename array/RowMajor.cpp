
#include <assert.h>
#include <math.h>
#include <string.h>
#include "RowMajor.h"
#include <iostream>
using namespace std;

RowMajor::RowMajor(const MDCoord &coord)
{
   assert(coord.nDim != 0);
   for (int k = 0; k < coord.nDim; k++)
      assert(coord.coords[k] > 0); // cannot have dimension length of 0
   this->dim = coord;
}

RowMajor::~RowMajor()
{
}

Key_t RowMajor::linearize(const MDCoord &coord)
{
   assert(coord.nDim == dim.nDim);

   Key_t key = 0;
   for (int k = 0; k < dim.nDim; k++)
   {
      assert(0 <= coord.coords[k] && coord.coords[k] < dim.coords[k]);
      key = key*dim.coords[k] + coord.coords[k];
   }
   return key;
}

MDCoord RowMajor::unlinearize(Key_t key)
{
   i64 coords[dim.nDim];
   for (int k = dim.nDim - 1; k >= 0; k--)
   {
      coords[k] = key % dim.coords[k];
      key /= dim.coords[k];
   }
   return MDCoord(coords, dim.nDim);
}

MDCoord RowMajor::move(const MDCoord &from, KeyDiff_t diff)
{
   assert(from.nDim == dim.nDim);
   for (int i = 0; i < dim.nDim; i++)
      assert(0 <= from.coords[i] && from.coords[i] < dim.coords[i]);

   MDCoord to(from);
   to.coords[dim.nDim - 1] += diff;
   if (to.coords[dim.nDim-1] == -1L)
       return to;
   for (int k = dim.nDim - 1; k > 0; k--)
   {
      if (0 <= to.coords[k] && to.coords[k] < dim.coords[k]) // carry-over done propogating
         break;

      to.coords[k-1] += to.coords[k]/dim.coords[k];
      if ((to.coords[k] %= dim.coords[k]) < 0) // if coords[k] is negative
      {
         to.coords[k] += dim.coords[k];
         to.coords[k-1]--;
      }
   }

   // the most significant bit
   if (to.coords[0] < 0 || to.coords[0] >= dim.coords[0])
   {
      if ((to.coords[0] %= dim.coords[0]) < 0)
         to.coords[0] += dim.coords[0];
   }

   return to;
}

RowMajor* RowMajor::clone()
{
   return new RowMajor(dim);
}

LinearizationType RowMajor::getType()
{
   return ROW;
}

bool RowMajor::equals(Linearization *l)
{
    if (typeid(*l) != typeid(*this))
        return false;
    RowMajor *ll = (RowMajor*) l;
    return ll->dim == this->dim;
}
