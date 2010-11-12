#ifndef MD_SPARSE_ITERATOR_H
#define MD_SPARSE_ITERATOR_H

#include "common/common.h"
#include "MDCoord.h"
#include "MDArray.h"
#include "Linearization.h"

/**
 * A class for iterating through a MDArray, skipping non-zero
 * elements.  A MDSparseIterator visits each <em>non-zero</em> element
 * in the range it is responsible for once and only once, in the order
 * as defined by Linearization.
 *
 * Each MDSparseIterator has a Linearization object associated with it
 * to help define the moveNext() and movePrev() operations.  Suppose
 * the linearization associated with the iterator can be viewed as a
 * function \f$ f: N^n \to N \f$, and the iterator currently points to
 * coordinate x inside the n-dim array.  After a call to moveNext(),
 * the iterator will point to coordinate y s.t. \f$ f(y)=f(x)+k \f$,
 * \f$ f(y)\neq 0 \f$, and \f$ k>0 \f$  and is minimal.
 * In other words, moveNext() moves the pointer to the next non-zero
 * entry.  Similarly, movePrev() moves the pointer to the previous
 * non-zero entry.
 *
 * Generally speaking, a sparse iterator may be as inefficient as
 * iterating through all elements like a dense iterator does and
 * returning only the non-zero elements.  For special cases where
 * acceleration is possible, subclasses should be created and used.
 *
 * \sa Iterator
 * \sa MDArray
 * \sa Linearization
 */

template<class nDim>
class MDSparseIterator: public MDArray<nDim>::Iterator
{
public:
    
    /**
     * Constructs an iterator given the associated array and
     * linearization order.  Caller should make sure param
     * linearization is not shared with other iterators.  It is
     * assumed that linearization is different from array's
     * linearization, and thus no optimizationis available.
     *
     * \param array A pointer to the associated MDArray.
     *
     * \param linearization A pointer to a Linearization object that
     * defines the iterating order.
     */
    MDSparseIterator(MDArray *array, Linearization *linearization);
    
    /**
     * Constructs a copy of the given iterator.
     *
     * \param src Source to be copied.
     */
    MDSparseIterator(const MDSparseIterator &src);

    /**
     * Gets the current coordinate and the datum the iterator points to.
     *
     * \param [out] coord The current coordinate.
     * \param [out] datum The current datum.
     */
    virtual void get(MDCoord &coord, Datum_t &datum);

    /**
     * Moves the iterator to the previous entry in the collection.
     * "Previous" is defined according to the linearization order.
     *
     * \return true if successfully moved to the previous entry; false
     * if already before the beginning of the collection.
     *
     */
    virtual bool movePrev();

    /**
     * Moves the iterator to the next entry in the collection.
     * "Next" is defined according to the linearization order.
     *
     * \return true if successfully moved to the next entry; false
     * if already after the end of the collection.
     *
     */
    virtual bool moveNext();

    /**
     * Replaces the current entry with the specified datum.
     * 
     * \param [out] datum The datum to be put.
     */
    virtual void put(const Datum_t &datum);

    /**
     * Virtual destructor.
     */
    virtual ~MDSparseIterator();

    /**
     * \copydoc Iterator::setRange()
     */
    virtual bool setRange(MDCoord &beginsAt, MDCoord &endsBy);

    virtual void reset();
    
protected:
    /// The array this iterator iterates through.
    MDArray *array;
    /// The Linearization associated with this iterator.
    Linearization *linearization;

    MDCoord beginsAt;
    MDCoord endsBy;
    MDCoord cursor;
};

#endif
