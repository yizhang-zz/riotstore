
#include <assert.h>
#include <math.h>
#include <string.h>
#include "BlockBased.h"


BlockBased::BlockBased(u8 nDims, const i64 *arraySizes, const i64 *blockSizes, const u8 *blockOrder, const u8 *microOrder)
{
   assert(nDims != 0);
   for (int i = 0; i < nDims; i++)
   {
      assert(arraySizes[i] != 0);
      assert(blockSizes[i] != 0 && blockSizes[i] < arraySizes[i]);
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
      microKey = microKey*blockSizes[k] + microCoords[k];
   }
   return blockKey*blockSize + microKey;
}

MDCoord BlockBased::unlinearize(Key_t key)
{
   Key_t blockKey = key / blockSize;
   Key_t microKey = key % blockSize;

   i64 coords[nDims];
   // i64 blockCoords[nDims];
   // i64 microCoords[nDims];
   for (int i = nDims - 1; i >= 0; i--)
   {
      u8 k = blockOrder[i];
      coords[i] = (blockKey % blocksPerArray[k])*blockSizes[i] + microKey % blockSizes[k];
      // blockCoords[i] = blockKey % blocksPerArray[k];
      blockKey /= blocksPerArray[k];
      // microCoords[i] = microKey % blockSizes[k];
      microKey /= blockSizes[k];
   }
   /*
   for (int i = 0; i < nDims; i++)
   {
      coords[i] = blockCoords[i]*blockSizes[i] + microCoords[i];
   }*/
   return MDCoord(coords, nDims);
}

MDCoord BlockBased::move(const MDCoord &from, KeyDiff_t diff)
{
}

BlockBased* BlockBased::clone()
{
   return new BlockBased(nDims, arraySizes, blockSizes, blockOrder,
         microOrder);
}

