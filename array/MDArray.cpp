
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "MDArray.h"
#include "../directly_mapped/DirectlyMappedArray.h"
#include "../btree/Btree.h"
#include "../btree/Splitter.h"
#include "MDDenseIterator.h"
#include "MDAccelDenseIterator.h"
#include "MDSparseIterator.h"
#include "MDAccelSparseIterator.h"
#include "BlockBased.h"
#include "ColMajor.h"
#include "RowMajor.h"

using namespace Btree;


#define OFFSET 1000

MDArray::MDArray(u8 nDim, i64 *dims, StorageType type, Linearization *lnrztn, const char *fileName)
{
   assert(nDim != 0);
   this->nDim = nDim;
   size = 1;
   for (int i = 0; i < nDim; i++)
   {
      assert(dims[i] > 0);
      size *= (u32)dims[i];
   }
   this->dims = new i64[nDim];
   memcpy(this->dims, dims, nDim*sizeof(i64));
   linearization = lnrztn->clone();
   char path[40]; // should be long enough..
   if (fileName == 0)
   {
      char temp[] = "MDArXXXXXX";
      int fd = mkstemp(temp);
      close(fd);
      strcpy(path, temp);
   }
   else
   {
      strcpy(path, fileName);
   }
//   MSplitter msp;
   switch (type)
   {
      case DMA:
         storage = new DirectlyMappedArray(path, size);
         break;
      case BTREE:
         // storage = new BTree(path, size, &msp, &msp); 
         break;
      default:
         storage = NULL;
         break;
   }
   PageHandle ph;
   storage->buffer->readPage(0, ph);
   int *header = (int*)((char*)ph + OFFSET);
   *header = type;
   *(header+1) = nDim;
   *(header+2) = linearization->getType();
   i64* header_dims = (i64*)(header + 3);
   memcpy(header_dims, dims, nDim*sizeof(i64));
   if (linearization->getType() == BLOCK)
   {
      memcpy(header_dims+nDim, ((BlockBased*)linearization)->blockDims, nDim*sizeof(i64));
      u8 *header_order = (u8*)(header_dims+2*nDim);
      memcpy(header_order, ((BlockBased*)linearization)->blockOrders, nDim*sizeof(u8));
      memcpy(header_order+nDim, ((BlockBased*)linearization)->microOrders, nDim*sizeof(u8));
   }
   storage->buffer->markPageDirty(ph);
   storage->buffer->unpinPage(ph);
}

MDArray::MDArray(const char *fileName)
{
   if (access(fileName, F_OK) != 0)
      throw ("File for array does not exist.");
   FILE *file = fopen(fileName, "r");
   fseek(file, OFFSET, SEEK_SET);
   int type, linType;
   fread(&type, sizeof(int), 1, file);
   fread(&nDim, sizeof(int), 1, file);
   fread(&linType, sizeof(int), 1, file);
   dims = new i64[nDim];
   fread(dims, sizeof(i64), nDim, file);
   size = 1;
   for (int i = 0; i < nDim; i++)
      size *= dims[nDim];
   if (linType == BLOCK)
   {
      i64 blockDims[nDim];
      u8 blockOrders[nDim];
      u8 microOrders[nDim];
      fread(blockDims, sizeof(i64), nDim, file);
      fread(blockOrders, sizeof(u8), nDim, file);
      fread(microOrders, sizeof(u8), nDim, file);
      linearization = new BlockBased(nDim, dims, blockDims, blockOrders,
            microOrders);
   }
   else if (linType == ROW)
   {
      linearization = new RowMajor(nDim, dims);      
   }
   else if (linType == COL)
   {
      linearization = new ColMajor(nDim, dims);
   }
   fclose(file);
   if (type == DMA)
   {
      storage = new DirectlyMappedArray(fileName, 0);
   }
   else if (type == BTREE)
   {
  //    MSplitter msp;
      // storage = new BTree(fileName, 0, &msp, &msp); 
   }
}

MDArray::~MDArray()
{
   delete[] dims;
   delete linearization;
   delete storage;
}

Linearization* MDArray::getLinearization()
{
   return linearization->clone(); // clone or not?
}

MDIterator* MDArray::createIterator(IteratorType t, Linearization *lnrztn)
{
   switch (t)
   {
      case Dense:
         return new MDDenseIterator(this, lnrztn);
      case Sparse:
         // return MDSparseIterator(storage, lnrztn);
      default:
         return NULL;
   }
}

MDIterator* MDArray::createNaturalIterator(IteratorType t)
{
   switch (t)
   {
      case Dense:
         return new MDAccelDenseIterator(this, createInternalIterator(Dense));
      case Sparse:
         // return MDAccelSparseIterator(storage, createInternalIterator(Sparse));
      default:
         return NULL;
   }
}

AccessCode MDArray::get(MDCoord &coord, Datum_t &datum)
{
   Key_t key = linearization->linearize(coord);
   if (storage->get(key, datum) == AC_OK)
      return AC_OK;
   return AC_OutOfRange;
}

AccessCode MDArray::put(MDCoord &coord, const Datum_t &datum)
{
   Key_t key = linearization->linearize(coord);
   if (storage->put(key, datum) == AC_OK)
      return AC_OK;
   return AC_OutOfRange;
}

ArrayInternalIterator* MDArray::createInternalIterator(IteratorType t)
{
    Key_t start = 0;
   return storage->createIterator(t, start, size);
}

