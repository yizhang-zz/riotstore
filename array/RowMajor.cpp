
#include <assert.h>
#include <math.h>
#include "RowMajor.h"


RowMajor::RowMajor(const MDCoord &coord)
{
   assert(coord.nDim != 0);
   for (int k = 0; k < coord.nDim; k++)
      assert(coord.coords[k] != 0); // cannot have dimension length of 0
   dimension = new MDCoord(coord);
}

RowMajor::~RowMajor()
{
   delete dimension;
}

Key_t RowMajor::linearize(const MDCoord &coord)
{
   assert(coord.nDim == dimension->nDim);
   assert(coord.coords[0] < dimension->coords[0]);
   Key_t key = coord.coords[0];
   for (int k = 1; k < dimension->nDim; k++)
   {
      assert(coord.coords[k] < dimension->coords[k]);
      key = key*dimension->coords[k] + coord.coords[k];
      // key *= dimension.coords[k];
      // key += coord[k];
   }
   return key;
}

MDCoord RowMajor::unlinearize(Key_t key)
{
   i64 coords[dimension->nDim];
   coords[dimension->nDim-1] = key % dimension->coords[dimension->nDim-1];
   for (int k = dimension->nDim - 2; k >= 0; k--)
   {
      key /= dimension->coords[k+1];
      coords[k] = key % dimension->coords[k];
   }
   return MDCoord(coords, dimension->nDim);
}

MDCoord RowMajor::move(const MDCoord &from, KeyDiff_t diff)
{
   return unlinearize(linearize(from) + diff);
/*   MDCoord to = MDCoord(from);
   if (diff > 0)
   {
      to += unlinearize(abs(diff));
      for (int k = dimension->nDim - 1; k > 0; k--)
      {
         if (to.coords[k] >= dimension->coords[k]) // overflow happens
         {
            to.coords[k] -= dimension->coords[k];
            to.coords[k-1]++;
         }
      }
   }
   else if (diff < 0)
   {
      to -= unlinearize(abs(diff));
      for (int k = dimension->nDim - 1; k < 0; k--)
      {
         if (to.coords[k] < 0) // overflow happens
         {
            to.coords[k] += dimension->coords[k];
            to.coords[k-1]--;
         }
      }
   }
   return to;*/
}

RowMajor* RowMajor::clone()
{
   return new RowMajor(*(this->dimension));
}

