#ifndef MDARRAY_H
#define MDARRAY_H

#include "../common/common.h"
#include "MDIterator.h"
#include "../lower/LinearStorage.h"
#include "../common/ArrayInternalIterator.h"
#include "Linearization.h"
#include <string>

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

class MDArray
{
protected:
    /// The underlying 1-D storage.
    LinearStorage *storage;
    /// The Linearization tied to the underlying 1-D storage.
    Linearization *linearization;
    MDCoord dim;
    std::string fileName;
    
public:
    static MDCoord peekDim(const char *fileName);
    
    u32 size; // number of elements
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
    MDArray(MDCoord &dim, StorageType type, Linearization *lrnztn, const char *fileName=0);

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
     * Destructor.
     */
    ~MDArray();

    /**
     * Gets the linearization of the array.
     *
     * \return Linearization.
     */
    Linearization * getLinearization();

    /**
     * Creates a new iterator with the specified linearization.
     *
     * \param t Type of iterator.
     * \param lnrztn The specified linearization for the iterator.
     * \return An iterator for this array.
     */
    MDIterator* createIterator(IteratorType t, Linearization *lnrztn);
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
    AccessCode get(MDCoord &coord, Datum_t &datum);

    /**
     * Puts an entry in the array.
     *
     * \param [in] coord Coordinate of the entry.
     * \param [in] datum Datum at coord.
     * \result OK if successful, OutOfRange if coord is out of range.
     */
    AccessCode put(MDCoord &coord, const Datum_t &datum);

    /**
     * Creates an internal iterator over the 1-D storage device.
     *
     * \param t Type of iterator.
     * \return An internal iterator.
     */
    ArrayInternalIterator* createInternalIterator(IteratorType t);
    
    const char* getFileName() { return fileName.c_str(); }
    int getNDim() { return dim.nDim; }
};

#endif
