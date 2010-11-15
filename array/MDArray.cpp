
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

using namespace std;
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

MDArray::MDArray(MDCoord &dim, Linearization *lnrztn, LeafSplitter *leaf, InternalSplitter *internal, const char *fileName)
    : leafsp(leaf), intsp(internal)
{
    allocatedSp = false;
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

    storage = new BTree(path, size, leafsp, intsp); 

    PageHandle ph;
    storage->buffer->readPage(0, ph);
    char *image = (char*)storage->buffer->getPageImage(ph);
    int *header = (int*)(image + OFFSET);
    *header = BTREE;
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
    this->fileName = path;
}

MDArray::MDArray(MDCoord &dim, StorageType type, Linearization *lnrztn, const char *fileName)
    : leafsp(NULL), intsp(NULL)
{
    allocatedSp = false;
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

    switch (type)
    {
    case DMA:
        storage = new DirectlyMappedArray(path, size);
        break;
    case BTREE:
        leafsp = new MSplitter<Datum_t>();
        intsp = new MSplitter<PID_t>();
        storage = new BTree(path, size, leafsp, intsp); 
        allocatedSp = true;
        break;
    default:
        storage = NULL;
        assert(false);
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
    this->fileName = path;
}

MDArray::MDArray(const char *fileName)
{
    allocatedSp = false;
    this->intsp = NULL;
    this->leafsp = NULL;
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
       //TODO: should read from file
       leafsp = new MSplitter<Datum_t>();
       intsp = new MSplitter<PID_t>();
       allocatedSp = true;
       storage = new BTree(fileName, leafsp, intsp);
   }
   this->fileName = fileName;
}

MDArray::~MDArray()
{
    if (allocatedSp)
    {
        delete intsp;
        delete leafsp;
    }
   delete linearization;
   delete storage;
}

Linearization* MDArray::getLinearization()
{
   return linearization;
}

MDIterator* MDArray::createIterator(IteratorType t, Linearization *lnrztn)
{
   switch (t)
   {
      case Dense:
         return new MDDenseIterator(this, lnrztn);
      case Sparse:
          assert(false);
          // return MDSparseIterator(storage, lnrztn);
   }
   return NULL;
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

AccessCode MDArray::get(Key_t &key, Datum_t &datum)
{
   if (storage->get(key, datum) == AC_OK)
      return AC_OK;
   return AC_OutOfRange;
}

AccessCode MDArray::batchGet(MDCoord &start, MDCoord &end, Datum_t *data)
{
    if (start.coords[0] >= dim.coords[0] || start.coords[1] >= dim.coords[1])
    {
        return AC_OutOfRange;
    }
    if (end.coords[0] >= dim.coords[0] || end.coords[1] >= dim.coords[1])
    {
        return AC_OutOfRange;
    }
    if (!(start.coords[0] <= end.coords[0] && start.coords[1] <= end.coords[1]))
    {
        return AC_OutOfRange;
    }

    i64 nRows = end.coords[0] - start.coords[0] + 1;
    i64 nCols = end.coords[1] - start.coords[1] + 1;
    i64 getCount = nRows * nCols;
    KVPair_t *gets = new KVPair_t[getCount];
    KVPair_t *curGet = gets;
    Datum_t *curDatum = data;

    for (i64 col = start.coords[1]; col <= end.coords[1]; col++)
    {
        for (i64 row = start.coords[0]; row <= end.coords[0]; row++)
        {
            curGet->key = linearization->linearize(MDCoord(2, row, col));
            curGet->datum = curDatum;
            curGet++;
            curDatum++;
        }
    }

    if (linearization->getType() != COL)
    {
        qsort(gets, getCount, sizeof(KVPair_t), compareKVPair);
    }

    int ac = storage->batchGet(getCount, gets);
    delete gets;
    if (ac == AC_OK)
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

AccessCode MDArray::put(Key_t &key, const Datum_t &datum)
{
   if (storage->put(key, datum) == AC_OK)
      return AC_OK;
   return AC_OutOfRange;
}

AccessCode MDArray::batchPut(MDCoord &start, MDCoord &end, Datum_t *data)
{
    if (end.coords[0] >= dim.coords[0] || end.coords[1] >= dim.coords[1])
    {
        return AC_OutOfRange;
    }
    if (!(start.coords[0] <= end.coords[0] && start.coords[1] <= end.coords[1]))
    {
        return AC_OutOfRange;
    }

    i64 nRows = end.coords[0] - start.coords[0] + 1;
    i64 nCols = end.coords[1] - start.coords[1] + 1;
    i64 putCount = nRows * nCols;
    KVPair_t *puts = new KVPair_t[putCount];
    KVPair_t *curPut = puts;
    Datum_t *curDatum = data;

    for (i64 col = start.coords[1]; col <= end.coords[1]; col++)
    {
        for (i64 row = start.coords[0]; row <= end.coords[0]; row++)
        {
            curPut->key = linearization->linearize(MDCoord(2, row, col));
            curPut->datum = curDatum;
            curPut++;
            curDatum++;
        }
    }

    if (linearization->getType() != COL)
    {
        qsort(puts, putCount, sizeof(KVPair_t), compareKVPair);
    }

    int ac = storage->batchPut(putCount, puts);
    delete puts;
    if (ac == AC_OK)
        return AC_OK;
    return AC_OutOfRange;
}

ArrayInternalIterator* MDArray::createInternalIterator(IteratorType t)
{
    Key_t start = 0;
    return storage->createIterator(t, start, size);
}
