#ifndef MDARRAY_H
#define MDARRAY_H

#include <string>
#include "common/common.h"
#include "common/ArrayInternalIterator.h"
#include "lower/LinearStorage.h"
#include "btree/Splitter.h"
#include "Linearization.h"
#include "SparseMatrix.h"

/**
 * A class for multi-dimensional arrays.  A MDArray presents a logical
 * n-D interface, i.e., access to any of its elements is through a
 * n-tuple which specifies the coordinates of that element inside the
 * array.  Internally, a MDArray is associated with a one-dimensional
 * storage.  The translation between the n-D appearance and the 1-D
 * internal representation is via Linearization.  Thus, each MDArray
 * object maintains a linearization for its storage device.
 *
 * Elements in a MDArray can be accessed directly by n-D coordinates,
 * or through Iterator objects.  Iterator only sees the logical n-D
 * interface of MDArray.  It is used to provide an easy way to
 * traverse a MDArray (or a subrange of it) according to some fixed
 * order.  The order is, not surprisingly, again specified by a
 * Linearization object.  It is important to realize that the
 * linearization (\f$ f \f$) of an iterator can be, and usually is,
 * different from the MDArray's internal storage linearization (\f$ g
 * \f$).  Take MDDenseIterator as example, in mathematical terms, the
 * the n-D coordinate of the current position in the MDArray is \f$ x
 * \f$, then the index of the next element (as defined by
 * MDDenseIterator) in the 1-D physical storage is \f$
 * f(g^{-1}(g(x)+1)) \f$.  MDSparseIterator is a little different, but
 * the concept of linearization is the same.
 *
 * A MDArray can be associated with multiple iterators, each
 * maintaining its own state.
 *
 * \sa MDCoord
 * \sa Iterator
 * \sa MDDenseIterator
 * \sa MDSparseIterator
 * \sa Linearization
 */

template<int nDim> 
class MDArray
{
public:
	typedef Iterator<MDCoord<nDim>, Datum_t> MDIterator;
	typedef MDCoord<nDim> Coord;
    //static Coord peekDim(const char *fileName);

    /**
     * Constructs a new MDArray with given dimensions.
     *
     * \param nDim Number of dimensions.
     * \param dims Array of sizes of each dimension.
     * \param type Type of storage that should be used.
     * \param fileName Name of the file in which the array should be
     * stored. If omitted, a random name consisting 10 hex digits is
     * generated. Guaranteed no existing file will be overwritten.
     */
    MDArray(const Coord &dim, StorageType type, Linearization<nDim> *lrnztn, const char *fileName=0);

    MDArray(const Coord &dim, Linearization<nDim> *lrnztn, Btree::LeafSplitter *leaf, Btree::InternalSplitter *internal, const char *fileName=0);
    /**
     * Constructs and initializes a MDArray from a file stored on
     * disk. ArrayStorage's factory method can analyze the file and
     * decide which type of storage the file contains. Dimension
     * information can alse be read from the file.
     *
     * \param fileName Name of file.
     */
    MDArray(const char *fileName);

	/**
	 * Copy constructor
	 */
	MDArray(const MDArray<nDim> &other);

    /**
     * Destructor.
     */
    ~MDArray();

    /**
     * Gets the linearization of the array.
     *
     * \return Linearization.
     */
    Linearization<nDim> * getLinearization();

    /**
     * Sets the linearization of the array.
     *
     * \return Linearization.
     */
    void setLinearization(Linearization<nDim>*);

    /**
     * Creates a new iterator with the specified linearization.
     *
     * \param t Type of iterator.
     * \param lnrztn The specified linearization for the iterator.
     * \return An iterator for this array.
     */
    MDIterator* createIterator(IteratorType t, Linearization<nDim> *lnrztn);
    //ArrayInternalIterator *getStorageIterator();

    /**
     * Creates a new iterator with "natural" linearization--same as
     * the one used by the underlying 1-D storage.  The returned
     * iterator supports accelerated operations.
     *
     * \param t Type of iterator.
     * \return A natural iterator.
     */
    MDIterator* createNaturalIterator(IteratorType t);

    /**
     * Gets an entry in the array.
     *
     * \param [in] coord Coordinate of the entry.
     * \param [out] datum Datum at coord.
     * \return OK if successful, OutOfRange if coord is out of range.
     */
    int get(const Coord &coord, Datum_t &datum) const;
    int get(const Key_t &key, Datum_t &datum) const;

    /**
      * Gets sub-array of entries
      *
      * \param [in] start Starting coordinate of sub-array.
      * \param [in] end Last coordinate of sub-array.
      * \param [out] data array of data values ut into array. NOTE, the order
      * of values in data should be in Col-Major order such that the first datum
      * should correspond to the value at start, and the last to the value at
      * end. The caller is also responsible for allocating enough space for data
      * to fit all coordinates between start and end.
      * \result OK if successful, OutOfRange is any coord within start and end
      * is out of range.
      */
	int batchGet(const Coord &begin, const Coord &end, Datum_t *data) const;

    /**
     * Puts an entry in the array.
     *
     * \param [in] coord Coordinate of the entry.
     * \param [in] datum Datum at coord.
     * \result OK if successful, OutOfRange if coord is out of range.
     */
    int put(const Coord &coord, const Datum_t &datum);
    int put(const Key_t &key, const Datum_t &datum);

    /**
      * Puts sub-array of entries into the array.
      *
      * \param [in] start Starting coordinate of sub-array.
      * \param [in] end Last coordinate of sub-array.
      * \param [in] data array of data values to put into array. NOTE, the order
      * of values in data should be in Col-Major order such that the first datum
      * should correspond to the value at start, and the last to the value at
      * end.
      * \result OK if successful, OutOfRange is any coord within start and end
      * is out of range.
      */
    int batchPut(const Coord &start, const Coord &end, Datum_t *data);

    /**
     * Creates an internal iterator over the 1-D storage device.
     *
     * \param t Type of iterator.
     * \return An internal iterator.
     */
    ArrayInternalIterator* createInternalIterator(IteratorType t);
    
    const char* getFileName() const { return fileName.c_str(); }
    int getNDim() const { return nDim; }
	Coord getDims() const { return dim; }
    double sparsity() const { return storage->sparsity(); }

	// math methods
	MDArray<nDim> & operator+=(const MDArray<nDim> &other);

protected:
    MDCoord<nDim> dim;
    u32 size; // number of elements
    /// The underlying 1-D storage.
    LinearStorage *storage;
    /// The Linearization tied to the underlying 1-D storage.
    Linearization<nDim> *linearization;
    std::string fileName;

    bool allocatedSp;
    Btree::LeafSplitter *leafsp;
    Btree::InternalSplitter *intsp;

    /**
     * Get all nonzero entries within the rectangle as defined by begin and
     * end, and linearize their coordinates using this->linearization. Entries
     * are sorted in increasing key order. v contains the result set upon return.
     */
	int batchGet(const Coord &begin, const Coord &end, std::vector<Entry> &v) const;
    /**
     * Put all (nonzero) entries in v into the array. Keys are calculated using
     * this->linearization and entries are in increasing key order.
     */
    int batchPut(MDArrayElement<nDim> *elements, int num);
};

class Matrix : public MDArray<2>
{
public:
    Matrix(const Coord &dim, StorageType type, Linearization<2> *lrnztn, const char *fileName=0)
		: MDArray<2>(dim, type, lrnztn, fileName)
	{
	}

    Matrix(const Coord &dim, Linearization<2> *lrnztn, Btree::LeafSplitter *leaf, Btree::InternalSplitter *internal, const char *fileName=0)
		: MDArray<2>(dim, lrnztn, leaf, internal, fileName)
	{
	}

    Matrix(const char *fileName) : MDArray<2>(fileName)
	{
	}

	Matrix(const Matrix &other) : MDArray<2>(other)
	{
	}

	Matrix operator*(const Matrix &other);

    using MDArray<2>::batchPut;
    using MDArray<2>::batchGet;
	SparseMatrix batchGet(const Coord &begin, const Coord &end) const;
    int batchPut(const Coord &begin, const SparseMatrix &);

};
#endif
