#include <string.h>
#include "MDArray.h"
#include "directly_mapped/DirectlyMappedArray.h"
#include "btree/Btree.h"
#include "btree/Splitter.h"
//#include "MDDenseIterator.h"
//#include "MDAccelDenseIterator.h"
//#include "MDSparseIterator.h"
//#include "MDAccelSparseIterator.h"
#include "BlockBased.h"
#include "ColMajor.h"
#include "RowMajor.h"

using namespace std;
using namespace Btree;


#define OFFSET 1000

/*
template<int nDim>
MDArray<nDim>::Coord MDArray<nDim>::peekDim(const char *fileName)
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
   
	Coord dim;
    fread(dim.coords, sizeof(i64), ndim, file);
    MDCoord dim = MDCoord(dims, ndim);
    delete[] dims;
    return dim;
}
*/
template<int nDim>
MDArray<nDim>::MDArray(MDCoord<nDim> &dim, Linearization<nDim> *lnrztn, LeafSplitter *leaf, InternalSplitter *internal, const char *fileName)
    : leafsp(leaf), intsp(internal)
{
    this->allocatedSp = false;
    this->dim = dim;
    assert(nDim != 0);
    this->size = 1;
    for (int i = 0; i < nDim; i++)
    {
        assert(dim[i] > 0);
        this->size *= (u32)dim[i];
    }
   
    //linearization = lnrztn->clone();
	linearization = lnrztn;

    char path[40]; // should be long enough
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
	BufferManager *buffer = storage->getBufferManager();
    buffer->readPage(0, ph);
    char *image = (char*)buffer->getPageImage(ph);
    int *header = (int*)(image + OFFSET);
    *header = BTREE;
    *(header+1) = nDim;
    *(header+2) = linearization->getType();
    i64* header_dims = (i64*)(header + 3);
    memcpy(header_dims, &dim[0], nDim*sizeof(i64));
    if (linearization->getType() == BLOCK)
    {
		BlockBased<nDim> *bl = static_cast<BlockBased<nDim>*>(linearization);
        memcpy(header_dims+nDim, bl->blockDims, nDim*sizeof(i64));
        u8 *header_order = (u8*)(header_dims+2*nDim);
        memcpy(header_order, bl->blockOrders, nDim*sizeof(u8));
        memcpy(header_order+nDim, bl->microOrders, nDim*sizeof(u8));
    }
    buffer->markPageDirty(ph);
    buffer->unpinPage(ph);
    this->fileName = path;
}

template<int nDim>
MDArray<nDim>::MDArray(MDCoord<nDim> &dim, StorageType type, Linearization<nDim> *lnrztn, const char *fileName)
    : leafsp(NULL), intsp(NULL)
{
    allocatedSp = false;
    this->dim = dim;
    assert(nDim != 0);
    size = 1;
    for (int i = 0; i < nDim; i++)
    {
        assert(dim[i] > 0);
        size *= (u32)dim[i];
    }
   
    //linearization = lnrztn->clone();
	linearization = lnrztn;

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
	BufferManager *buffer = storage->getBufferManager();
    buffer->readPage(0, ph);
    char *image = (char*)buffer->getPageImage(ph);
    int *header = (int*)(image + OFFSET);
    *header = type;
    *(header+1) = nDim;
    *(header+2) = linearization->getType();
    i64* header_dims = (i64*)(header + 3);
    memcpy(header_dims, &dim[0], nDim*sizeof(i64));
    if (linearization->getType() == BLOCK)
    {
		BlockBased<nDim> *bl = static_cast<BlockBased<nDim>*>(linearization);
        memcpy(header_dims+nDim, bl->blockDims, nDim*sizeof(i64));
        u8 *header_order = (u8*)(header_dims+2*nDim);
        memcpy(header_order, bl->blockOrders, nDim*sizeof(u8));
        memcpy(header_order+nDim, bl->microOrders, nDim*sizeof(u8));
    }
    buffer->markPageDirty(ph);
    buffer->unpinPage(ph);
    this->fileName = path;
}

template<int nDim>
MDArray<nDim>::MDArray(const char *fileName)
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
	assert(ndim == nDim);
	fread(&linType, sizeof(int), 1, file);

	fread(&dim[0], sizeof(i64), nDim, file);

	size = 1;
	for (int i = 0; i < nDim; i++)
		size *= (u32) dim[i];

	if (linType == BLOCK)
	{
		i64 *blockDims = new i64[nDim];
		u8 *blockOrders = new u8[nDim];
		u8 *microOrders = new u8[nDim];
		fread(blockDims, sizeof(i64), nDim, file);
		fread(blockOrders, sizeof(u8), nDim, file);
		fread(microOrders, sizeof(u8), nDim, file);
		linearization = new BlockBased<nDim>(&dim[0], blockDims,
				blockOrders, microOrders);
		delete[] blockDims;
		delete[] blockOrders;
		delete[] microOrders;
	}
	else if (linType == ROW)
	{
		linearization = new RowMajor<nDim>(&dim[0]);   
	}
	else if (linType == COL)
	{
		linearization = new ColMajor<nDim>(&dim[0]);
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

template<int nDim>
MDArray<nDim>::~MDArray()
{
    if (allocatedSp)
    {
        delete intsp;
        delete leafsp;
    }
   //delete linearization;
   delete storage;
}

template<int nDim>
Linearization<nDim>* MDArray<nDim>::getLinearization()
{
   return linearization;
}

template<int nDim>
void MDArray<nDim>::setLinearization(Linearization<nDim>* nl)
{
	linearization = nl;
}

template<int nDim>
typename MDArray<nDim>::MDIterator *MDArray<nDim>::createIterator(IteratorType t, Linearization<nDim> *lnrztn)
{
   switch (t)
   {
      case Dense:
         //return new MDDenseIterator(this, lnrztn);
      case Sparse:
          assert(false);
          // return MDSparseIterator(storage, lnrztn);
   }
   return NULL;
}

template<int nDim>
typename MDArray<nDim>::MDIterator *MDArray<nDim>::createNaturalIterator(IteratorType t)
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

template<int nDim>
AccessCode MDArray<nDim>::get(const MDCoord<nDim> &coord, Datum_t &datum) const
{
   Key_t key = linearization->linearize(coord);
   if (storage->get(key, datum) == AC_OK)
      return AC_OK;
   return AC_OutOfRange;
}

template<int nDim>
AccessCode MDArray<nDim>::get(const Key_t &key, Datum_t &datum) const
{
   if (storage->get(key, datum) == AC_OK)
      return AC_OK;
   return AC_OutOfRange;
}

template<int nDim>
AccessCode MDArray<nDim>::put(const MDCoord<nDim> &coord, const Datum_t &datum)
{
   Key_t key = linearization->linearize(coord);
   if (storage->put(key, datum) == AC_OK)
      return AC_OK;
   return AC_OutOfRange;
}

template<int nDim>
AccessCode MDArray<nDim>::put(const Key_t &key, const Datum_t &datum)
{
   if (storage->put(key, datum) == AC_OK)
      return AC_OK;
   return AC_OutOfRange;
}

template<int nDim>
AccessCode MDArray<nDim>::batchPut(const Coord &start, const Coord &end, const Datum_t *data)
{
    if (start[0] >= dim[0] || start[1] >= dim[1])
    {
        return AC_OutOfRange;
    }
    if (end[0] >= dim[0] || end[1] >= dim[1])
    {
        return AC_OutOfRange;
    }
    if (!(start[0] <= end[0] && start[1] <= end[1]))
    {
        return AC_OutOfRange;
    }

    i64 nRows = end[0] - start[0] + 1;
    i64 nCols = end[1] - start[1] + 1;
    i64 putCount = nRows * nCols;
    KVPair_t *puts = new KVPair_t[putCount];
    KVPair_t *curPut = puts;
    const Datum_t *curDatum = data;

    for (i64 row = start[0]; row <= end[0]; row++)
    {
        for (i64 col = start[1]; col <= end[1]; col++)
        {
            curPut->key = linearization->linearize(Coord(row, col));
            curPut->datum = curDatum;
            curPut++;
            curDatum++;
        }
    }

    if (linearization->getType() != ROW)
    {
        qsort(puts, putCount, sizeof(KVPair_t), compareKVPair);
    }

    int ac = storage->batchPut(putCount, puts);
    delete puts;
    if (ac == AC_OK)
        return AC_OK;
    return AC_OutOfRange;
}

template<int nDim>
ArrayInternalIterator* MDArray<nDim>::createInternalIterator(IteratorType t)
{
    //Key_t start = 0;
	return NULL;
    //return storage->createIterator(t, start, size);
}


template class MDArray<1>;
template class MDArray<2>;
template class MDArray<3>;

