
#include <assert.h>
#include <math.h>
#include <string.h>
#include "RowMajor.h"

RowMajor::RowMajor(u8 nDim, const i64* coords)
{
   assert(nDim != 0);
   for (int k = 0; k < nDim; k++)
      assert(coords[k] > 0); // cannot have dimension length of 0

   this->nDim = nDim;
   dims = new i64[nDim];
   memcpy(dims, coords, nDim*sizeof(i64));
}

RowMajor::RowMajor(const MDCoord &coord)
{
   assert(coord.nDim != 0);
   for (int k = 0; k < coord.nDim; k++)
      assert(coord.coords[k] > 0); // cannot have dimension length of 0

   nDim = coord.nDim;
   dims = new i64[nDim];
   memcpy(dims, coord.coords, nDim*sizeof(i64));
}

RowMajor::~RowMajor()
{
   delete[] dims;
}

Key_t RowMajor::linearize(const MDCoord &coord)
{
   assert(coord.nDim == nDim);

   Key_t key = 0;
   for (int k = 0; k < nDim; k++)
   {
      assert(0 <= coord.coords[k] && coord.coords[k] < dims[k]);
      key = key*dims[k] + coord.coords[k];
   }
   return key;
}

MDCoord RowMajor::unlinearize(Key_t key)
{
   i64 coords[nDim];
   for (int k = nDim - 1; k >= 0; k--)
   {
      coords[k] = key % dims[k];
      key /= dims[k];
   }
   return MDCoord(coords, nDim);
}

MDCoord RowMajor::move(const MDCoord &from, KeyDiff_t diff)
{
   assert(from.nDim == nDim);
   for (int i = 0; i < nDim; i++)
      assert(0 <= from.coords[i] && from.coords[i] < dims[i]);

   MDCoord to(from);
   to.coords[nDim - 1] += diff;
   for (int k = nDim - 1; k > 0; k--)
   {
      if (0 <= to.coords[k] && to.coords[k] < dims[k]) // carry-over done propogating
         break;

      to.coords[k-1] += to.coords[k]/dims[k];
      if ((to.coords[k] %= dims[k]) < 0) // if coords[k] is negative
      {
         to.coords[k] += dims[k];
         to.coords[k-1]--;
      }
   }

   // the most significant bit
   if (to.coords[0] < 0 || to.coords[0] >= dims[0])
   {
      if ((to.coords[0] %= dims[0]) < 0)
         to.coords[0] += dims[0];
   }

   return to;
}

RowMajor* RowMajor::clone()
{
   return new RowMajor(nDim, dims);
}

LinearizationType RowMajor::getType()
{
   return linType;
}
