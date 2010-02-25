
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

using namespace Btree;

MDArray::MDArray(u32 nDim, u32 *dims, StorageType type, const char *fileName)
{
   assert(nDim != 0);
   this->nDim = nDim;
   size = 1;
   for (int i = 0; i < nDim; i++)
   {
      assert(dims[i] > 0);
      size *= dims[i];
   }
   this->dims = new i64[nDim];
   memcpy(this->dims, dims, nDim*sizeof(i64));
   linearization = NULL;
   // TODO: generate random hex filename if fileName=0
   // Remember to add break for cases in switch
   MSplitter msp;
   switch (type)
   {
      case DMA:
         storage = new DirectlyMappedArray(fileName, 0);
         break;
      case BTREE:
         //storage = new BTree(fileName, 0, &msp, &msp); 
         break;
      default:
         storage = NULL;
         break;
   }
}

MDArray::MDArray(const char *fileName)
{
   // TODO: determine type of underlying storage
   linearization = NULL;
}

MDArray::~MDArray()
{
   delete[] dims;
   delete linearization;
   delete storage;
}

void MDArray::setLinearization(Linearization *lnrztn)
{
   if (linearization != NULL)
      delete linearization;
   linearization = lnrztn->clone();
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

