
#include <assert.h>
#include <math.h>
#include <string.h>
#include "BlockBased.h"


BlockBased::BlockBased(u8 nDim, const i64 *arrayDims, const i64 *blockDims, const u8 *blockOrders, const u8 *microOrders)
{
    // only vector or matrix supported for now
   assert(nDim <= 2);
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
   this->remainders = new i64[nDim];
   blockSize = 1;
   blocksPerArray = new i64[nDim];
   for (int i = 0; i < nDim; i++)
   {
      blockSize *= blockDims[i];
      blocksPerArray[i] = arrayDims[i] / blockDims[i];
      remainders[i] = arrayDims[i] % blockDims[i];
   }
}

BlockBased::~BlockBased()
{
   delete[] arrayDims;
   delete[] blockDims;
   delete[] blockOrders;
   delete[] microOrders;
   delete[] blocksPerArray;
   delete[] remainders;
   //delete[] units;
}

Key_t BlockBased::linearize(const MDCoord &coord)
{
   assert(coord.nDim == nDim);
   //for (int i = 0; i < nDim; i++)
   //   assert(0 <= coord.coords[i] && coord.coords[i] < arrayDims[i]);

   if (nDim == 1) {
       return (Key_t)coord.coords[0];
   }

   i64 blockCoords[nDim];
   i64 microCoords[nDim];
   i64 blockBegin[nDim];
   for (int i = 0; i < nDim; i++)
   {
      //assert(coord.coords[i] < arrayDims[i]);
      blockCoords[i] = coord.coords[i] / blockDims[i];
      microCoords[i] = coord.coords[i] % blockDims[i];
      blockBegin[i] = blockCoords[i] * blockDims[i];
   }

   i64 blockKey = 0;
   i64 microKey = 0;
   i64 unit = 1;
   i64 munit = 1;
   u8 k = blockOrders[0];
   u8 l = blockOrders[1];
   //blockKey += blockCoords[k]*blockDims[k]*units[k];
   blockKey += blockCoords[k]*blockDims[k]*arrayDims[l];
   if (remainders[k]!=0 && blocksPerArray[k]==blockCoords[k]) {
        blockKey += blockDims[l]*remainders[k]*blockCoords[l];
   }
   else
        blockKey += blockDims[l]*blockDims[k]*blockCoords[l];
   // inside block
   u8 i = microOrders[0];
   u8 j = microOrders[1];
   if (remainders[j]!=0 && blocksPerArray[j]==blockCoords[j])
       blockKey += microCoords[i]*remainders[j]+microCoords[j];
   else
       blockKey += microCoords[i]*blockDims[j]+microCoords[j];
   return (Key_t)blockKey;

   for (int i = nDim-1; i >=0; i--)
   {
      u8 k = blockOrders[i];
      blockKey += unit* blockBegin[k];
      unit *= arrayDims[k];
      u8 j = microOrders[i];
      microKey += munit * microCoords[j];
      // if array dim is not exact multiple of block dim
      // and it's the last block in that dim
      if (remainders[j] != 0 
              && blockCoords[k] == blocksPerArray[k])
          munit *= remainders[j];
      else
          munit *= blockDims[j];
   }
   return (Key_t)(blockKey + microKey);
}

MDCoord BlockBased::unlinearize(Key_t key)
{
   if (nDim == 1) {
       return MDCoord(1, key);
   }

   i64 blockCoords[nDim];
   i64 microCoords[nDim];
   u8 k = blockOrders[0];
   u8 l = blockOrders[1];
   blockCoords[k] = key/arrayDims[l]/blockDims[k];
   Key_t temp = key - blockCoords[k]*blockDims[k]*arrayDims[l];
   if (remainders[k] != 0 && blocksPerArray[k]==blockCoords[k]) {
       blockCoords[l] = temp/blockDims[l]/remainders[k];
       temp -= blockDims[l]*remainders[k]*blockCoords[l];
   }
   else {
       blockCoords[l] = temp/blockDims[l]/blockDims[k];
       temp -= blockDims[l]*blockDims[k]*blockCoords[l];
   }
   // inside block
   u8 i = microOrders[0];
   u8 j = microOrders[1];
   if (remainders[j]!=0 && blocksPerArray[j]==blockCoords[j]) {
       microCoords[i] = temp/remainders[j];
       microCoords[j] = temp - microCoords[i]*remainders[j];
   }
   else {
       microCoords[i] = temp/blockDims[j];
       microCoords[j] = temp - microCoords[i]*blockDims[j];
   }
   return MDCoord(2, blockCoords[0]*blockDims[0]+microCoords[0],
           blockCoords[1]*blockDims[1]+microCoords[1]);

   Key_t blockKey = key / blockSize;
   Key_t microKey = key % blockSize;

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
    return unlinearize(linearize(from)+diff);
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

Linearization *BlockBased::transpose()
{
    i64 ad[nDim];
    for (int i=0; i<nDim; i++)
        ad[i] = arrayDims[nDim-1-i];
    i64 bd[nDim];
    for (int i=0; i<nDim; i++)
        bd[i] = blockDims[nDim-1-i];
    u8 bo[nDim];
    for (int i=0; i<nDim; i++)
        bo[i] = blockOrders[nDim-1-i];
    u8 mo[nDim];
    for (int i=0; i<nDim; i++)
        mo[i] = microOrders[nDim-1-i];
    return new BlockBased(nDim, ad, bd, bo, mo);
}

void BlockBased::getBlock(Key_t key, Key_t &begin, Key_t &end)
{
   i64 blockCoords[nDim];
   i64 microCoords[nDim];
   u8 k = blockOrders[0];
   u8 l = blockOrders[1];
   blockCoords[k] = key/arrayDims[l]/blockDims[k];
   Key_t temp = key - blockCoords[k]*blockDims[k]*arrayDims[l];
   if (remainders[k] != 0 && blocksPerArray[k]==blockCoords[k]) {
       blockCoords[l] = temp/blockDims[l]/remainders[k];
       temp -= blockDims[l]*remainders[k]*blockCoords[l];
   }
   else {
       blockCoords[l] = temp/blockDims[l]/blockDims[k];
       temp -= blockDims[l]*blockDims[k]*blockCoords[l];
   }
   begin = linearize(MDCoord(2, blockCoords[0]*blockDims[0],
               blockCoords[1]*blockDims[1]));
   if (blockCoords[l]==blocksPerArray[l]) {
       blockCoords[l] = 0;
       blockCoords[k]++;
       if (blockCoords[k]>blocksPerArray[k]) { // end of entire array
           end = arrayDims[0]*arrayDims[1]-1;
           return;
       }
   }
   else {
       blockCoords[l]++;
   }
   end = linearize(MDCoord(2, blockCoords[0]*blockDims[0],
               blockCoords[1]*blockDims[1]))-1;
}

void BlockBased::getBlock(Key_t key, MDCoord &begin, MDCoord &end)
{
   i64 blockCoords[nDim];
   i64 microCoords[nDim];
   u8 k = blockOrders[0];
   u8 l = blockOrders[1];
   blockCoords[k] = key/arrayDims[l]/blockDims[k];
   Key_t temp = key - blockCoords[k]*blockDims[k]*arrayDims[l];
   if (remainders[k] != 0 && blocksPerArray[k]==blockCoords[k]) {
       blockCoords[l] = temp/blockDims[l]/remainders[k];
       temp -= blockDims[l]*remainders[k]*blockCoords[l];
   }
   else {
       blockCoords[l] = temp/blockDims[l]/blockDims[k];
       temp -= blockDims[l]*blockDims[k]*blockCoords[l];
   }
   microOrders[0] = microOrders[1] = 0; // begining of the block
   begin = (MDCoord(2, blockCoords[0]*blockDims[0],
               blockCoords[1]*blockDims[1]));
   if (blockCoords[l]==blocksPerArray[l]) {
       blockCoords[l] = 0;
       blockCoords[k]++;
   }
   else {
       blockCoords[l]++;
   }
   end = (MDCoord(2, blockCoords[0]*blockDims[0],
               blockCoords[1]*blockDims[1]));
}
