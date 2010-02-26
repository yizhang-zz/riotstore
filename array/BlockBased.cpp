
#include <assert.h>
#include <math.h>
#include <string.h>
#include "BlockBased.h"


BlockBased::BlockBased(u8 nDim, const i64 *arrayDims, const i64 *blockDims, const u8 *blockOrders, const u8 *microOrders)
{
   assert(nDim != 0);
   for (int i = 0; i < nDim; i++)
   {
      assert(arrayDims[i] > 0);
      assert(0 < blockDims[i] && blockDims[i] <= arrayDims[i]);
      assert(0 <= blockOrders[i] && blockOrders[i] < nDim); // duplicate check?
      assert(0 <= microOrders[i] && microOrders[i] < nDim); // duplicate check?
   }

   this->nDim = nDim;
   this->arrayDims = new i64[nDim];
   memcpy(this->arrayDims, arrayDims, nDim*sizeof(i64));
   this->blockDims = new i64[nDim];
   memcpy(this->blockDims, blockDims, nDim*sizeof(i64));
   this->blockOrders = new u8[nDim];
   memcpy(this->blockOrders, blockOrders, nDim*sizeof(u8));
   this->microOrders = new u8[nDim];
   memcpy(this->microOrders, microOrders, nDim*sizeof(u8));
   blockSize = 1;
   blocksPerArray = new i64[nDim];
   for (int i = 0; i < nDim; i++)
   {
      blockSize *= blockDims[i];
      blocksPerArray[i] = arrayDims[i] / blockDims[i];
   }
}

BlockBased::~BlockBased()
{
   delete[] arrayDims;
   delete[] blockDims;
   delete[] blockOrders;
   delete[] microOrders;
   delete[] blocksPerArray;
}

Key_t BlockBased::linearize(const MDCoord &coord)
{
   assert(coord.nDim == nDim);
   for (int i = 0; i < nDim; i++)
      assert(0 <= coord.coords[i] && coord.coords[i] < arrayDims[i]);

   i64 blockCoords[nDim];
   i64 microCoords[nDim];
   for (int i = 0; i < nDim; i++)
   {
      assert(coord.coords[i] < arrayDims[i]);
      blockCoords[i] = coord.coords[i] / blockDims[i];
      microCoords[i] = coord.coords[i] % blockDims[i];
   }

   Key_t blockKey = 0;
   Key_t microKey = 0;
   for (int i = 0; i < nDim; i++)
   {
      u8 k = blockOrders[i];
      blockKey = blockKey*blocksPerArray[k] + blockCoords[k];
      u8 j = microOrders[i];
      microKey = microKey*blockDims[j] + microCoords[j];
   }
   return (Key_t)blockKey*blockSize + microKey;
}

MDCoord BlockBased::unlinearize(Key_t key)
{
   Key_t blockKey = key / blockSize;
   Key_t microKey = key % blockSize;

   i64 blockCoords[nDim];
   i64 microCoords[nDim];
   for (int i = nDim - 1; i >= 0; i--)
   {
      u8 k = blockOrders[i];
      blockCoords[k] = (blockKey % blocksPerArray[k]) * blockDims[k];
      blockKey /= blocksPerArray[k];

      u8 j = microOrders[i];
      microCoords[j] = microKey % blockDims[j];
      microKey /= blockDims[j];
   }

   for (int i = 0; i < nDim; i++)
   {
      blockCoords[i] += microCoords[i];
   }
   return MDCoord(blockCoords, nDim);
}
/*
   assert(from.nDim == nDim);
   MDCoord to(from);
   to.coords[nDim - 1] += diff;
   for (int k = nDim - 1; k >= 0; k--)
   {
      if (to.coords[k] < dimSizes[k]) // carry-over done propogating
         break;
      if (k > 0)
         to.coords[k-1] += to.coords[k]/dimSizes[k];
      to.coords[k] %= dimSizes[k];
   }
   return to;
*/
MDCoord BlockBased::move(const MDCoord &from, KeyDiff_t diff)
{
   assert(from.nDim == nDim);
   for (int i = 0; i < nDim; i++)
      assert(0 <= from.coords[i] && from.coords[i] < arrayDims[i]);

   MDCoord to(from);
   i64 blockCoords[nDim];
   i64 microCoords[nDim];
   KeyDiff_t blockDiff = diff / blockSize;
   KeyDiff_t microDiff = diff % blockSize;

   for (int i = 0; i < nDim; i++)
   {
      blockCoords[i] = from.coords[i] / blockDims[i];
      microCoords[i] = from.coords[i] % blockDims[i];
   }

   microCoords[microOrders[nDim - 1]] += microDiff;
   for (int i = nDim - 1; i >= 0; i--)
   {
      u8 k = microOrders[i];
      if (0 <= microCoords[k] && microCoords[k] < blockDims[k])
         break;

      if (i > 0)
         microCoords[microOrders[i-1]] += microCoords[k] / blockDims[k];
      else
         blockDiff += microCoords[k] / blockDims[k];

      if ((microCoords[k] %= blockDims[k]) < 0) // check negative index
      {
         microCoords[k] += blockDims[k];
         if (i > 0)
            microCoords[microOrders[i-1]]--;
         else
            blockDiff--;
      }
   }

   blockCoords[blockOrders[nDim - 1]] += blockDiff;
   for (int i = nDim -1; i >= 0; i--)
   {
      u8 k = blockOrders[i];
      if (0 <= blockCoords[k] && blockCoords[k] < blocksPerArray[k])
         break;

      if (i > 0)
         blockCoords[blockOrders[i-1]] += blockCoords[k] / blocksPerArray[k];

      if ((blockCoords[k] %= blocksPerArray[k]) < 0) // check negative index
      {
         blockCoords[k] += blocksPerArray[k];
         if (i > 0)
            blockCoords[blockOrders[i-1]]--;
      }
   }
 
   for (int i = 0; i < nDim; i++)
   {
      microCoords[i] += blockCoords[i] * blockDims[i];
   }

   return MDCoord(microCoords, nDim);

  // return unlinearize(linearize(from) + diff);
}

Linearization* BlockBased::clone()
{
   return new BlockBased(nDim, arrayDims, blockDims, blockOrders,
         microOrders);
}

bool BlockBased::equals(Linearization *l)
{
    if (typeid(*this)!=typeid(*l))
        return false;
    BlockBased *ll = (BlockBased*)l;
    return (nDim==ll->nDim && isSameArray(nDim,arrayDims,ll->arrayDims)
            && isSameArray(nDim,blockDims,ll->blockDims)
            && isSameArray(nDim, blockOrders, ll->microOrders));
}

LinearizationType BlockBased::getType()
{
   return BLOCK;
}
