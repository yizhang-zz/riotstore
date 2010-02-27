
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "MDArray.h"
#include "../directly_mapped/DirectlyMappedArray.h"
#include "../btree/Btree.h"
#include "../btree/Splitter.h"
#include "MDDenseIterator.h"
//#include "MDAccelDenseIterator.h"
#include "MDSparseIterator.h"
//#include "MDAccelSparseIterator.h"
#include "BlockBased.h"
#include "ColMajor.h"
#include "RowMajor.h"

using namespace Btree;


#define OFFSET 1000

MDCoord MDArray::peekDim(const char *fileName)
{
    if (access(fileName, F_OK) != 0)
        throw ("File for array does not exist.");
    FILE *file = fopen(fileName, "rb");
    // first page in file is used by PagedStorageContainer
    // the header page of the LinearStorage is actually the 2nd page
    fseek(file, PAGE_SIZE+OFFSET, SEEK_SET);
    int type, ndim, linType;
    fread(&type, sizeof(int), 1, file);
    fread(&ndim, sizeof(int), 1, file);
    fread(&linType, sizeof(int), 1, file);
   
    i64 *dims = new i64[ndim];
    fread(dims, sizeof(i64), ndim, file);
    MDCoord dim = MDCoord(dims, ndim);
    delete[] dims;
    return dim;
}

MDArray::MDArray(MDCoord &dim, StorageType type, Linearization *lnrztn, const char *fileName)
{
    this->dim = dim;
    assert(dim.nDim != 0);
    size = 1;
    for (int i = 0; i < dim.nDim; i++)
    {
        assert(dim.coords[i] > 0);
        size *= (u32)dim.coords[i];
    }
   
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
    char *image = (char*)storage->buffer->getPageImage(ph);
    int *header = (int*)(image + OFFSET);
    *header = type;
    *(header+1) = dim.nDim;
    *(header+2) = linearization->getType();
    i64* header_dims = (i64*)(header + 3);
    memcpy(header_dims, dim.coords, dim.nDim*sizeof(i64));
    if (linearization->getType() == BLOCK)
    {
        memcpy(header_dims+dim.nDim, ((BlockBased*)linearization)->blockDims, dim.nDim*sizeof(i64));
        u8 *header_order = (u8*)(header_dims+2*dim.nDim);
        memcpy(header_order, ((BlockBased*)linearization)->blockOrders, dim.nDim*sizeof(u8));
        memcpy(header_order+dim.nDim, ((BlockBased*)linearization)->microOrders, dim.nDim*sizeof(u8));
    }
    storage->buffer->markPageDirty(ph);
    storage->buffer->unpinPage(ph);
}

MDArray::MDArray(const char *fileName)
{
   if (access(fileName, F_OK) != 0)
      throw ("File for array does not exist.");
   FILE *file = fopen(fileName, "rb");
   // first page in file is used by PagedStorageContainer
   // the header page of the LinearStorage is actually the 2nd page
   fseek(file, PAGE_SIZE+OFFSET, SEEK_SET);
   int type, ndim, linType;
   fread(&type, sizeof(int), 1, file);
   fread(&ndim, sizeof(int), 1, file);
   fread(&linType, sizeof(int), 1, file);
   
   i64 *dims = new i64[ndim];
   fread(dims, sizeof(i64), ndim, file);
   dim = MDCoord(dims, ndim);
   delete[] dims;
   
   size = 1;
   for (int i = 0; i < dim.nDim; i++)
      size *= dim.coords[i];
   
   if (linType == BLOCK)
   {
      i64 *blockDims = new i64[dim.nDim];
      u8 *blockOrders = new u8[dim.nDim];
      u8 *microOrders = new u8[dim.nDim];
      fread(blockDims, sizeof(i64), dim.nDim, file);
      fread(blockOrders, sizeof(u8), dim.nDim, file);
      fread(microOrders, sizeof(u8), dim.nDim, file);
      linearization = new BlockBased(dim.nDim, dim.coords, blockDims,
                                     blockOrders, microOrders);
      delete[] blockDims;
      delete[] blockOrders;
      delete[] microOrders;
   }
   else if (linType == ROW)
   {
      linearization = new RowMajor(dim);      
   }
   else if (linType == COL)
   {
      linearization = new ColMajor(dim);
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
   delete linearization;
   delete storage;
}

Linearization* MDArray::getLinearization()
{
   return linearization; // clone or not?
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
          //return new MDAccelDenseIterator(this, createInternalIterator(Dense));
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

