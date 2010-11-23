#include <string.h>
#include <gsl/gsl_cblas.h>
#include "common/Config.h"
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
MDArray<nDim>::MDArray(const MDCoord<nDim> &dim, Linearization<nDim> *lnrztn, LeafSplitter *leaf, InternalSplitter *internal, const char *fileName)
    : leafsp(leaf), intsp(internal)
{
    this->allocatedSp = false;
    this->dim = dim;
	// The linearized space can be larger than what dim indicates;
	// e.g., when 2x2 blocks are used for a 3x3 array, the linearized
	// space will be of size 4x4=16. Thus the linear storage should
	// be able to hold at least this many records.
	Coord coord = lnrztn->getActualDims();
	size = 1;
	for (int i=0; i<nDim; ++i)
		size *= (u32) coord[i];
   
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
MDArray<nDim>::MDArray(const MDCoord<nDim> &d, StorageType type, Linearization<nDim> *lnrztn, const char *fileName)
    : leafsp(NULL), intsp(NULL)
{
    allocatedSp = false;
    dim = d;
	Coord coord = lnrztn->getActualDims();
	size = 1;
	for (int i=0; i<nDim; ++i)
		size *= (u32) coord[i];
   
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
	Coord coord = linearization->getActualDims();
	size = 1;
	for (int i=0; i<nDim; ++i)
		size *= (u32) coord[i];
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
AccessCode MDArray<nDim>::batchPut(const Coord &start, const Coord &end, Datum_t *data)
{
	// start should be within the array's bound,
	// but end could be out, either beyond the right bound or lower bound.
	for (int i=0; i<nDim; ++i)
		if (start[i] >= dim[i])
			return AC_OutOfRange;

    i64 nRows = end[0] - start[0] + 1;
    i64 nCols = end[1] - start[1] + 1;
    i64 putCount = nRows * nCols;
    KVPair_t *puts = new KVPair_t[putCount];
    KVPair_t *curPut = puts;
    Datum_t *curDatum = data;

	for (i64 col = start[1]; col <= end[1]; col++)
    {
		for (i64 row = start[0]; row <= end[0]; row++)
        {
            curPut->key = linearization->linearize(Coord(row, col));
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
    delete[] puts;
    if (ac == AC_OK)
        return AC_OK;
    return AC_OutOfRange;
}

template<int nDim>
AccessCode MDArray<nDim>::batchGet(const Coord &start, const Coord &end, Datum_t *data) const
{
	// start should be within the array's bound,
	// but end could be out, either beyond the right bound or lower bound.
	for (int i=0; i<nDim; ++i)
		if (start[i] >= dim[i])
			return AC_OutOfRange;

    i64 nRows = end[0] - start[0] + 1;
    i64 nCols = end[1] - start[1] + 1;
    i64 getCount = nRows * nCols;
    KVPair_t *gets = new KVPair_t[getCount];
    KVPair_t *curGet = gets;
    Datum_t *curDatum = data;

	for (i64 col = start[1]; col <= end[1]; col++)
    {
		for (i64 row = start[0]; row <= end[0]; row++)
        {
            curGet->key = linearization->linearize(Coord(row, col));
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
    delete[] gets;
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

template<int nDim>
MDArray<nDim> & MDArray<nDim>::operator+=(const MDArray<nDim> &other)
{
	// Safe even if other==this
	// Use this' linearization
	LinearizationType type = this->linearization->getType();
	Coord blockDims;
	i64 blockSize;
	// Assumes at least one operand has block based layout
	if (type & BLOCK) {
		BlockBased<nDim> *bl = static_cast<BlockBased<nDim>*>(linearization);
		blockDims = bl->getBlockDims();
		blockSize = bl->getBlockSize();
	} else if (other.linearization->getType() & BLOCK) {
		BlockBased<nDim> *bl = static_cast<BlockBased<nDim>*>(other.linearization);
		blockDims = bl->getBlockDims();
		blockSize = bl->getBlockSize();
	}

	i64 temp[nDim] = {0};
	Coord begin = Coord(temp);
	for (int i=0; i<nDim; ++i)
		temp[i] = 1;
	Coord box = blockDims - Coord(temp); // bounds are inclusive
	Coord end = begin + box;
	Datum_t *leftop = new Datum_t[blockSize];
	Datum_t *rightop = new Datum_t[blockSize];
	while (true) {
		for (int i=nDim-1; i>0; --i) {
			if (begin[i]>=dim[i]) {
				begin[i-1] += blockDims[i-1];
				begin[i] = 0;
				end[i] = begin[i] + box[i];
				end[i-1] += blockDims[i-1];
			}
			else
				break;
		}
		if (begin[0] >= dim[0])
			break;
		// retrive, compute, and write back
		this->batchGet(begin, end, leftop);
		other.batchGet(begin, end, rightop);
		for (i64 i=0; i<blockSize; ++i)
			leftop[i] += rightop[i];
		this->batchPut(begin, end, leftop);

		// move to next block
		begin[nDim-1] += box[nDim-1]+1;
		end[nDim-1]   += box[nDim-1]+1;
	}
	delete[] leftop;
	delete[] rightop;
	return *this;
}

template class MDArray<1>;
template class MDArray<2>;
template class MDArray<3>;


Matrix Matrix::operator*(const Matrix &other)
{
	if (dim[1] != other.dim[0])
		throw "Invalid argument";

	// Safe even if other==this
	// Use this' linearization
	LinearizationType type = this->linearization->getType();
	// square block
	Coord blockDims(1000,1000);
	i64 blockSize = 1000*1000;

	i64 retDims[] = {dim[0], other.dim[1]};
	u8 orders[]   = {1, 0};
	static BlockBased<2> retLin(retDims, &blockDims[0], orders, orders); 
	static MSplitter<PID_t> msp;// = new MSplitter<PID_t>;
	static BSplitter<Datum_t> bsp(config->denseLeafCapacity);// = new BSplitter<Datum_t>
	//Matrix ret(Coord(retDims), &retLin, &bsp, &msp);
	Matrix ret(Coord(retDims), DMA, &retLin);

	i64 temp[2] = {0};
	Coord begin = Coord(temp);
	temp[0] = temp[1] = 1;
	Coord box = blockDims - Coord(temp); // bounds are inclusive
	Coord end = begin + box;
	Datum_t *leftop = new Datum_t[blockSize];
	Datum_t *rightop = new Datum_t[blockSize];
	Datum_t *result = new Datum_t[blockSize];
	int nbr = 1+(dim[0]-1)/blockDims[0];
	int nbc = 1+(other.dim[1]-1)/blockDims[1];
	int nbx = 1+(dim[1]-1)/blockDims[1];
	for (int i=0; i<nbr; ++i) {
		for (int j=0; j<nbc; ++j) {
			double beta = 0.0;
			for (int k=0; k<nbx; ++k) {
				// read block[i,k] from this and block[k,j] from other
				begin[0] = i*blockDims[0];
				begin[1] = k*blockDims[1];
				end[0]   = begin[0] + blockDims[0] - 1;
				end[1]   = begin[1] + blockDims[1] - 1;
				this->batchGet(begin, end, leftop);
				begin[0] = k*blockDims[0];
				begin[1] = j*blockDims[1];
				end[0]   = begin[0] + blockDims[0] - 1;
				end[1]   = begin[1] + blockDims[1] - 1;
				other.batchGet(begin, end, rightop);
				cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans,
						blockDims[0], // #rows of leftop
						blockDims[1], // #cols of rightop
						blockDims[1], // #cols of leftop or #rows of rightop
						1, // alpha, scaling factor of the product
						leftop,
						blockDims[0], // first dimension (row) of leftop
						rightop,
						blockDims[0], // first dimension (row) of rightop
						beta, // beta, scaling factor of the constant
						result, // the constant input, will be written as output
						blockDims[0] //first dimension (row) of result
						);
				beta = 1.0; // result will be accmulated
			}
			begin[0] = i*blockDims[0];
			begin[1] = j*blockDims[1];
			end[0]   = begin[0] + blockDims[0] - 1;
			end[1]   = begin[1] + blockDims[1] - 1;
			ret.batchPut(begin, end, result);
		}
	}

	delete[] leftop;
	delete[] rightop;
	delete[] result;
	return ret;
}
