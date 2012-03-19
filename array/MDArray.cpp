#include <string.h>
extern "C" {
#include <cblas.h>
};
#include <fstream>
#include "common/Config.h"
#include "MDArray.h"
#include "directly_mapped/DirectlyMappedArray.h"
#include "btree/Btree.h"
#include "btree/Splitter.h"
#include "BlockBased.h"
#include "ColMajor.h"
#include "RowMajor.h"
#include "btree/ExternalSort.h"

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
MDArray<nDim>::MDArray(const StorageParam *sp, const MDCoord<nDim> &d, Linearization<nDim> *lnrztn): dim(d), storage(NULL)
{
    // The linearized space can be larger than what dim indicates;
    // e.g., when 2x2 blocks are used for a 3x3 array, the linearized
    // space will be of size 4x4=16. Thus the linear storage should
    // be able to hold at least this many records.
    Coord coord = lnrztn->getActualDims();
    typename Coord::Coord size_ = 1;
    for (int i=0; i<nDim; ++i)
        size_ *= coord[i];
    size = (size_t) size_;
   
    linearization = lnrztn->clone();
    setStorage(sp);
}

template<int nDim>
MDArray<nDim>::MDArray(const MDCoord<nDim> &d, Linearization<nDim> *lnrztn): dim(d), storage(NULL)
{
    Coord coord = lnrztn->getActualDims();

    typename Coord::Coord size_ = 1;
    for (int i=0; i<nDim; ++i)
        size_ *= coord[i];
    size = (size_t) size_;
   
    linearization = lnrztn->clone();
}

template<int nDim>
void MDArray<nDim>::setStorage(const StorageParam *sp)
{
    createStorage(sp);

    PageHandle ph;
    BufferManager *buffer = storage->getBufferManager();
    buffer->readPage(0, ph);
    char *image = ph->getImage();
    char *header = image + OFFSET;
	dim.serialize(&header);
	linearization->serialize(&header);
    ph->markDirty();
	ph->unpin();
}

// Create an MDArray from existing file by parsing file content
template<int nDim>
MDArray<nDim>::MDArray(const char *fileName): fileName(fileName)
{
    if (access(fileName, F_OK) != 0)
        throw ("File for array does not exist.");

	storage = LinearStorage::fromFile(fileName);

    PageHandle ph;
    BufferManager *buffer = storage->getBufferManager();
    buffer->readPage(0, ph);
    char *image = ph->getImage();
	ph->unpin();
    char *header = image + OFFSET;
	dim = MDCoord<nDim>::parse(&header);
	linearization = Linearization<nDim>::parse(&header);

    Coord coord = linearization->getActualDims();
    typename Coord::Coord size_ = 1;
    for (int i=0; i<nDim; ++i)
        size_ *= coord[i];
    size = (size_t) size_;
}

template<int nDim>
MDArray<nDim>::MDArray(const MDArray<nDim> &other)
{
    dim = other.dim;
    size = other.size;
    linearization = other.linearization->clone();

    char path[40] = "mdaXXXXXX";
    mktemp(path);
    fileName = path;

    // flush other and copy its file
    other.storage->flush();
    fstream in(other.fileName.c_str(), fstream::in|fstream::binary);
    in << noskipws;
    istream_iterator<char> begin(in);
    istream_iterator<char> end;
    fstream out(path, fstream::out|fstream::trunc|fstream::binary);
    ostream_iterator<char> begin2(out);
    copy(begin, end, begin2);
    in.close();
    out.close();

	storage = LinearStorage::fromFile(path);
}


// this->size must already be initialized!
template<int nDim>
void MDArray<nDim>::createStorage(const StorageParam *sp)
{
    if (storage)
        delete storage;

	char path[40] = "mdaXXXXXX";
	if (!sp->fileName)
		mktemp(path);
	else
		strcpy(path, sp->fileName);
	this->fileName = path;

    switch(sp->type) {
    case DMA:
        storage = new DirectlyMappedArray(path, size);
        break;
    case BTREE:
        storage = new BTree(path, size, sp->leafSp,
                sp->intSp, sp->useDenseLeaf);
        break;
    default:
        Error("unknown storage type");
    }
}

// for batch put
template<int nDim>
MDArray<nDim>::MDArray(const StorageParam *sp, Linearization<nDim> *lnrztn,
					   const char *parserType,
					   const char* inputFileName,
					   int bufferSize) : storage(NULL)
{
    if (!inputFileName || access(inputFileName, F_OK) != 0)
		throw ("File for array does not exist.");

    Parser<nDim> *parser = Parser<nDim>::createParser(parserType, inputFileName);

    dim = parser->dim();
    linearization = lnrztn->clone();
	// lnrztn may come with bogus dim info; replace it with the real dim
    linearization->setDims(dim);

    Coord actualDims = linearization->getActualDims();

    typename Coord::Coord size_ = 1;
    for (int i=0; i<nDim; ++i)
        size_ *= actualDims[i];
    size = (size_t) size_;

    createStorage(sp);

    ExternalSort sorter(bufferSize);
    Coord *inputCoord = new Coord[bufferSize];
    Entry *entries = new Entry[bufferSize];
    Datum_t *data = new Datum_t[bufferSize];

    int parsedCount = bufferSize;
    while(parsedCount == bufferSize){
        parsedCount = parser->parse(bufferSize, inputCoord, data);
        if (parsedCount == 0)
            break;
        for (int i=0; i<parsedCount; i++) {
            entries[i].key = linearization->linearize(inputCoord[i]);
            entries[i].datum = data[i];
        }
        sorter.streamToChunk(entries, parsedCount);
    }

    int batchSize = bufferSize; // could be different
    int sortedCount = batchSize;
    while (sortedCount == batchSize){
      sortedCount = sorter.mergeSortToStream(entries, batchSize);
      if (sortedCount == 0)
          break;
      storage->batchPut(sortedCount, entries);
    }
    delete[] entries;
    delete[] data;
    delete[] inputCoord;
    delete parser;

    PageHandle ph;
    BufferManager *buffer = storage->getBufferManager();
    buffer->readPage(0, ph);
    char *image = ph->getImage();
    char *header = image + OFFSET;
	dim.serialize(&header);
	linearization->serialize(&header);
    ph->markDirty();
	ph->unpin();
}



template<int nDim>
MDArray<nDim>::~MDArray()
{
    if (storage)
        delete storage;
    delete linearization;
}

template<int nDim>
Linearization<nDim>* MDArray<nDim>::getLinearization()
{
   return linearization;
}

template<int nDim>
void MDArray<nDim>::setLinearization(Linearization<nDim>* nl)
{
    delete linearization;
    linearization = nl->clone();
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
int MDArray<nDim>::get(const MDCoord<nDim> &coord, Datum_t &datum) const
{
   Key_t key = linearization->linearize(coord);
   if (storage->get(key, datum) == AC_OK)
      return AC_OK;
   return AC_OutOfRange;
}

template<int nDim>
int MDArray<nDim>::get(const Key_t &key, Datum_t &datum) const
{
   if (storage->get(key, datum) == AC_OK)
      return AC_OK;
   return AC_OutOfRange;
}

template<int nDim>
int MDArray<nDim>::put(const MDCoord<nDim> &coord, const Datum_t &datum)
{
   Key_t key = linearization->linearize(coord);
   if (storage->put(key, datum) == AC_OK)
      return AC_OK;
   return AC_OutOfRange;
}

template<int nDim>
int MDArray<nDim>::put(const Key_t &key, const Datum_t &datum)
{
   if (storage->put(key, datum) == AC_OK)
      return AC_OK;
   return AC_OutOfRange;
}

template<int nDim>
int MDArray<nDim>::batchPut(const Coord &start, const Coord &end, Datum_t *data)
{
    using namespace std;
    Datum_t *p = data;
    vector<Segment> *segments = linearization->getOverlap(begin, end);
    for (vector<Segment>::iterator it = segments->begin();
         it != segments->end();
         ++it) {
        // segment is inclusive: [it->begin, it->end]
        // but storage->batchGet is exclusive
        Key_t len = it->end - it->begin + 1;
        storage->batchPut(it->begin, it->end+1, p);
        p += len;
    }
    delete segments;
    return AC_OK;
#if 0
    // start should be within the array's bound,
    // but end could be out, either beyond the right bound or lower bound.
    for (int i=0; i<nDim; ++i)
        if (start[i] >= dim[i])
            return AC_OutOfRange;

    i64 nRows = end[0] - start[0] + 1;
    i64 nCols = end[1] - start[1] + 1;
    i64 putCount = nRows * nCols;
    Entry *puts = new Entry[putCount];

    // walk in msd -> lsd order (because of column order)
    Coord coord(start);
    int j = 0;
    while (true) {
        for (int i=0; i<nDim-1; ++i) {
            if (coord[i] == end[i]+1) { // reset and carry
                coord[i] = start[i];
                coord[i+1]++;
            }
            else
                break;
        }
        if (coord[nDim-1] == end[nDim-1]+1)
            break;
        puts[j].key = linearization->linearize(coord);
        puts[j].datum = data[j];
        j++;
        coord[0]++;
    }

    if (linearization->getType() != COL)
        std::sort(puts, puts+putCount);

    int ac = storage->batchPut(putCount, puts);
    delete[] puts;
    if (ac == AC_OK)
        return AC_OK;
    return AC_OutOfRange;
#endif
}

template<int nDim>
int MDArray<nDim>::batchPut(std::vector<MDArrayElement<nDim> > &elements)
{
	int num = elements.size();
    Entry *entries = new Entry[num];
    for (int i=0; i<num; ++i) {
        entries[i].key = linearization->linearize(elements[i].coord);
        entries[i].datum = elements[i].datum;
    }
    std::sort(entries, entries+num);
    int ac = storage->batchPut(num, entries);
    delete[] entries;
    return ac;
}

template<int nDim>
int MDArray<nDim>::batchGet(const Coord &begin, const Coord &end,
                            Datum_t *data) const
{
    using namespace std;
    Datum_t *p = data;
    vector<Segment> *segments = linearization->getOverlap(begin, end);
    for (vector<Segment>::iterator it = segments->begin();
         it != segments->end();
         ++it) {
        // segment is inclusive: [it->begin, it->end]
        // but storage->batchGet is exclusive
        Key_t len = it->end - it->begin + 1;
        storage->batchGet(it->begin, it->end+1, p);
        p += len;
    }
    delete segments;
    return AC_OK;
    /*
    // start should be within the array's bound,
    // but end could be out, either beyond the right bound or lower bound.
    for (int i=0; i<nDim; ++i)
        if (start[i] >= dim[i])
            return AC_OutOfRange;

    i64 nRows = end[0] - start[0] + 1;
    i64 nCols = end[1] - start[1] + 1;
    i64 getCount = nRows * nCols;
    Entry *gets = new Entry[getCount];

    // walk in msd -> lsd order (because of column order)
    Coord coord(start);
    int j = 0;
    while (true) {
        for (int i=0; i<nDim-1; ++i) {
            if (coord[i] == end[i]+1) { // reset and carry
                coord[i] = start[i];
                coord[i+1]++;
            }
            else
                break;
        }
        if (coord[nDim-1] == end[nDim-1]+1)
            break;
        gets[j].key = linearization->linearize(coord);
        gets[j].pdatum = data+j;
        j++;
        coord[0]++;
    }

    if (linearization->getType() != COL)
        std::sort(gets, gets+getCount);

    int ac = storage->batchGet(getCount, gets);
    delete[] gets;
    if (ac == AC_OK)
        return AC_OK;
    return AC_OutOfRange;
    */
}

template<int nDim>
int MDArray<nDim>::batchGet(const Coord &begin, const Coord &end,
                            std::vector<MDArrayElement<nDim> > &v) const
{
	using namespace std;
	vector<Segment> *segments = linearization->getOverlap(begin, end);
	for (vector<Segment>::iterator it = segments->begin();
			it != segments->end();
			++it) {
		std::vector<Entry> ve;
		// segment is inclusive: [it->begin, it->end]
		// but storage->batchGet is exclusive
		storage->batchGet(it->begin, it->end+1, ve);
		for (vector<Entry>::iterator eit = ve.begin();
				eit != ve.end(); ++eit) {
			MDArrayElement<nDim> elem;
			elem.coord = linearization->unlinearize(eit->key);
			elem.datum = eit->datum;
			v.push_back(elem);
		}
	}
	//storage->getBufferManager()->printStat();
	delete segments;
	return AC_OK;
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
    i64 blockSize = 0;
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
        batchGet(begin, end, leftop);
        other.batchGet(begin, end, rightop);
        for (i64 i=0; i<blockSize; ++i)
            leftop[i] += rightop[i];
        batchPut(begin, end, leftop);

        // move to next block
        begin[nDim-1] += box[nDim-1]+1;
        end[nDim-1]   += box[nDim-1]+1;
    }
    delete[] leftop;
    delete[] rightop;
    return *this;
}


//template class MDArray<1>;
template class MDArray<2>;

SparseMatrix Matrix::batchGet(const Coord &begin, const Coord &end) const
{
    std::vector<MDArrayElement<2> > v;
    MDArray<2>::batchGet(begin, end, v);  // v contains all non-zero records
    SparseMatrix ret(v, begin, end, false);
    return ret;
}

int Matrix::batchPut(const Coord &begin, const SparseMatrix &sm)
{
    //int *ap = (int*) (array)->p;
    //int *ai = (int*) (array)->i;
    //double *ax = (double*) (array)->x;
    //size_t ncol = array->ncol;
    //int size = ap[ncol];
    //Entry *puts = new Entry[size];
    //int k = 0;
    //for (int i=0; i<ncol; ++i) {
    //    for (int j=ap[i]; j<ap[i+1]; ++j) {
    //        puts[k].key =linearization->linearize(Coord(ai[j]+begin[0], i+begin[1]));
    //        puts[k].datum = ax[j];
    //        k++;
    //    }
    //}
    size_t size = sm.size();
    Entry *puts = new Entry[size];
    size_t k = 0;
    SparseMatrix::Iterator it = sm.begin();
    for (; it != sm.end(); ++it) {
        SparseMatrix::Element e = *it;
        puts[k].key = linearization->linearize(e.coord+begin);
        puts[k].datum = e.datum;
        k++;
    }
    assert(k == size);
    if (linearization->getType() != COL)
        std::sort(puts, puts+size);
    int ret = storage->batchPut(size, puts);
    delete[] puts;
    return ret;
}

Matrix Matrix::operator*(const Matrix &other)
{
    return multiply(other, config->matmulBlockFactor,
                    config->matmulBlockFactor,
                    config->matmulBlockFactor);
}

Matrix Matrix::multiply(const Matrix &other, i64 bf1, i64 bf2, i64 bf3)
{
    if (dim[1] != other.dim[0])
        throw "Invalid argument";

    // Safe even if other==this
    // Use this' linearization

    // Param 0: blocking factor of left and right operator
    Coord blockl(bf1, bf2);
    Coord blockr(bf2, bf3);
    assert(blockl[1]==blockr[0]);
    Coord block(blockl[0], blockr[1]);
 
    // (int+double)x < double*x => x<.667
    bool sparseA = sparsity() < .667;
    bool sparseB = other.sparsity() < .667;

    Datum_t *leftop = NULL;
    Datum_t *rightop = NULL;
    Datum_t *result = NULL;
    if (!sparseA)
        leftop = new Datum_t[blockl[0]*blockl[1]];
    if (!sparseB)
        rightop = new Datum_t[blockr[0]*blockr[1]];
    if (!sparseA || !sparseB)
        result = new Datum_t[blockl[0]*blockr[1]];

    i64 retDims[] = {dim[0], other.dim[1]};
    u8 orders[]   = {1, 0}; // result should be column major?
    BlockBased<2> retLin(retDims, &block[0], orders, orders); 
    MSplitter<PID_t> msp(false);
    // Param 1: what is the boundary value? dense leaf cap or sparse leaf cap?
    BSplitter<Datum_t> bsp(config->useDenseLeaf);
    // Param 2: what storage type to use? dma or btree?
    Matrix ret(Coord(retDims), &retLin);
    StorageParam param;
    param.fileName = NULL;
    if (storage->type() == DMA && other.storage->type() == DMA) {
        param.type = DMA;
        ret.setStorage(&param);
    }
    else {
        param.type = BTREE;
        param.leafSp = config->leafSplitter;
        param.intSp = config->internalSplitter;
        param.useDenseLeaf = config->useDenseLeaf;
        ret.setStorage(&param);
    }

    Coord begin, end;
    int nbr = 1+(dim[0]-1)/blockl[0];
    int nbc = 1+(other.dim[1]-1)/blockr[1];
    int nbx = 1+(dim[1]-1)/blockl[1];
    for (int i=0; i<nbr; ++i) {
        for (int j=0; j<nbc; ++j) {
            switch(sparseA*2+sparseB) {
            case 0: // both dense
                {
                    double beta = 0.0;
                    for (int k=0; k<nbx; ++k) {
                        // read block[i,k] from this and block[k,j] from other
                        begin[0] = i*blockl[0];
                        begin[1] = k*blockl[1];
                        end = begin + blockl - Coord(1, 1);
                        MDArray<2>::batchGet(begin, end, leftop);
                        begin[0] = k*blockr[0];
                        begin[1] = j*blockr[1];
                        end = begin + blockr - Coord(1, 1);
                        other.MDArray<2>::batchGet(begin, end, rightop);
                        cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans,
                                blockl[0], // #rows of leftop
                                blockr[1], // #cols of rightop
                                blockl[1], // #cols of leftop or #rows of rightop
                                1, // alpha, scaling factor of the product
                                leftop,
                                blockl[0], // first dimension (row) of leftop
                                rightop,
                                blockr[0], // first dimension (row) of rightop
                                beta, // beta, scaling factor of the constant
                                result, // the constant input, will be written as output
                                blockl[0] //first dimension (row) of result
                                );
                        beta = 1.0; // result will be accmulated
                    }
                    begin[0] = i*block[0];
                    begin[1] = j*block[1];
                    end = begin + block - Coord(1, 1);
                    ret.MDArray<2>::batchPut(begin, end, result);
                }
                break;
            case 1: // this dense, other sparse
                break;
            case 2: // this sparse, other dense, use chomod_sdmult, result dense
                {
                    cholmod_dense resultd, rightd;
                    resultd.nrow = blockl[0];
                    resultd.ncol = blockr[1];
                    resultd.nzmax = resultd.nrow * resultd.ncol;
                    resultd.d = resultd.nrow;
                    resultd.x = result;
                    resultd.xtype = CHOLMOD_REAL;
                    resultd.dtype = CHOLMOD_DOUBLE;
                    rightd.nrow = blockr[0];
                    rightd.ncol = blockr[1];
                    rightd.nzmax = blockr[0] * blockr[1];
                    rightd.d = rightd.nrow;
                    rightd.x = rightop;
                    rightd.xtype = CHOLMOD_REAL;
                    rightd.dtype = CHOLMOD_DOUBLE;
                    double alpha[] = {1, 0};
                    double beta[]  = {0, 0};
                    for (int k=0; k<nbx; ++k) {
                        begin[0] = i*blockl[0];
                        begin[1] = k*blockl[1];
                        end = begin + blockl - Coord(1, 1);
                        SparseMatrix lop = batchGet(begin, end);
                        // check lop begin
                        // check lop end
                        begin[0] = k*blockr[0];
                        begin[1] = j*blockr[1];
                        end = begin + blockr - Coord(1, 1);
                        other.batchGet(begin, end, rightop);
                        // check rightop begin
                        // check rightop end
                        // result = alpha ( sparse * dense) + beta result
                        cholmod_sdmult(
                                lop.storage(), // cholmod_sparse
                                0, // no transpose
                                alpha, // scaling factor for sparse matrix
                                beta, // scaling factor for result
                                &rightd,
                                &resultd,
                                SparseMatrix::chmCommon);
                        beta[0] = 1; // result will be accumulated
                    }
                    begin[0] = i*block[0];
                    begin[1] = j*block[1];
                    end = begin + block - Coord(1, 1);
                    ret.batchPut(begin, end, result);
                }
                break;
            case 3: // both sparse, use cholmod_ssmult, result sparse
                {
                    SparseMatrix resultSp(block[0], block[1]);
                    for (int k=0; k<nbx; ++k) {
                        // read block[i,k] from this and block[k,j] from other
                        begin[0] = i*blockl[0];
                        begin[1] = k*blockl[1];
                        end = begin + blockl - Coord(1, 1);
                        SparseMatrix lop = batchGet(begin, end);
                        begin[0] = k*blockr[0];
                        begin[1] = j*blockr[1];
                        end = begin + blockr - Coord(1, 1);
                        SparseMatrix rop = other.batchGet(begin, end);
                        resultSp += lop * rop;
                    }
                    begin[0] = i*block[0];
                    begin[1] = j*block[1];
                    ret.batchPut(begin, resultSp);
                }
                break;
            }
        } // for j
    } // for i

    if (!sparseA)
        delete[] leftop;
    if (!sparseB)
        delete[] rightop;
    if (!sparseA || !sparseB)
        delete[] result;
    return ret;
}

