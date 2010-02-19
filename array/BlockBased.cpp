
#include <assert.h>
#include <math.h>
#include <string.h>
#include "BlockBased.h"


BlockBased::BlockBased(u8 nDims, const i64 *arraySizes, const i64 *blockSizes, const u8 *blockOrder, const u8 *microOrder)
{
   assert(nDims != 0);
   for (int i = 0; i < nDims; i++)
   {
      assert(arraySizes[i] > 0);
      assert(0 < blockSizes[i] && blockSizes[i] <= arraySizes[i]);
      assert(0 <= blockOrder[i] && blockOrder[i] < nDims); // duplicate check?
      assert(0 <= microOrder[i] && microOrder[i] < nDims); // duplicate check?
   }

   this->nDims = nDims;
   this->arraySizes = new i64[nDims];
   memcpy(this->arraySizes, arraySizes, nDims*sizeof(i64));
   this->blockSizes = new i64[nDims];
   memcpy(this->blockSizes, blockSizes, nDims*sizeof(i64));
   this->blockOrder = new u8[nDims];
   memcpy(this->blockOrder, blockOrder, nDims*sizeof(u8));
   this->microOrder = new u8[nDims];
   memcpy(this->microOrder, microOrder, nDims*sizeof(u8));
   blockSize = 1;
   blocksPerArray = new i64[nDims];
   for (int i = 0; i < nDims; i++)
   {
      blockSize *= blockSizes[i];
      blocksPerArray[i] = arraySizes[i] / blockSizes[i];
   }
}

BlockBased::~BlockBased()
{
   delete[] arraySizes;
   delete[] blockSizes;
   delete[] blockOrder;
   delete[] microOrder;
   delete[] blocksPerArray;
}

Key_t BlockBased::linearize(const MDCoord &coord)
{
   assert(coord.nDims == nDims);
   for (int i = 0; i < nDims; i++)
      assert(0 <= coord.coords[i] && coord.coords[i] < arraySizes[i]);

   i64 blockCoords[nDims];
   i64 microCoords[nDims];
   for (int i = 0; i < nDims; i++)
   {
      assert(coord.coords[i] < arraySizes[i]);
      blockCoords[i] = coord.coords[i] / blockSizes[i];
      microCoords[i] = coord.coords[i] % blockSizes[i];
   }

   Key_t blockKey = 0;
   Key_t microKey = 0;
   for (int i = 0; i < nDims; i++)
   {
      u8 k = blockOrder[i];
      blockKey = blockKey*blocksPerArray[k] + blockCoords[k];
      u8 j = microOrder[i];
      microKey = microKey*blockSizes[j] + microCoords[j];
   }
   return (Key_t)blockKey*blockSize + microKey;
}

MDCoord BlockBased::unlinearize(Key_t key)
{
   Key_t blockKey = key / blockSize;
   Key_t microKey = key % blockSize;

   i64 blockCoords[nDims];
   i64 microCoords[nDims];
   for (int i = nDims - 1; i >= 0; i--)
   {
      u8 k = blockOrder[i];
      blockCoords[k] = (blockKey % blocksPerArray[k]) * blockSizes[k];
      blockKey /= blocksPerArray[k];

      u8 j = microOrder[i];
      microCoords[j] = microKey % blockSizes[j];
      microKey /= blockSizes[j];
   }

   for (int i = 0; i < nDims; i++)
   {
      blockCoords[i] += microCoords[i];
   }
   return MDCoord(blockCoords, nDims);
}
/*
   assert(from.nDims == nDims);
   MDCoord to(from);
   to.coords[nDims - 1] += diff;
   for (int k = nDims - 1; k >= 0; k--)
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
   assert(from.nDims == nDims);
   for (int i = 0; i < nDims; i++)
      assert(0 <= from.coords[i] && from.coords[i] < arraySizes[i]);

   MDCoord to(from);
   i64 blockCoords[nDims];
   i64 microCoords[nDims];
   KeyDiff_t blockDiff = diff / blockSize;
   KeyDiff_t microDiff = diff % blockSize;

   for (int i = 0; i < nDims; i++)
   {
      blockCoords[i] = from.coords[i] / blockSizes[i];
      microCoords[i] = from.coords[i] % blockSizes[i];
   }

   microCoords[microOrder[nDims - 1]] += microDiff;
   for (int i = nDims - 1; i >= 0; i--)
   {
      u8 k = microOrder[i];
      if (0 <= microCoords[k] && microCoords[k] < blockSizes[k])
         break;

      if (i > 0)
         microCoords[microOrder[i-1]] += microCoords[k] / blockSizes[k];
      else
         blockDiff += microCoords[k] / blockSizes[k];

      if ((microCoords[k] %= blockSizes[k]) < 0) // check negative index
      {
         microCoords[k] += blockSizes[k];
         if (i > 0)
            microCoords[microOrder[i-1]]--;
         else
            blockDiff--;
      }
   }

   blockCoords[blockOrder[nDims - 1]] += blockDiff;
   for (int i = nDims -1; i >= 0; i--)
   {
      u8 k = blockOrder[i];
      if (0 <= blockCoords[k] && blockCoords[k] < blocksPerArray[k])
         break;

      if (i > 0)
         blockCoords[blockOrder[i-1]] += blockCoords[k] / blocksPerArray[k];

      if ((blockCoords[k] %= blocksPerArray[k]) < 0) // check negative index
      {
         blockCoords[k] += blocksPerArray[k];
         if (i > 0)
            blockCoords[blockOrder[i-1]]--;
      }
   }
 
   for (int i = 0; i < nDims; i++)
   {
      microCoords[i] += blockCoords[i] * blockSizes[i];
   }

   return MDCoord(microCoords, nDims);

  // return unlinearize(linearize(from) + diff);
}

BlockBased* BlockBased::clone()
{
   return new BlockBased(nDims, arraySizes, blockSizes, blockOrder,
         microOrder);
}

