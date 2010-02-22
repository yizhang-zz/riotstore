#ifndef MD_DENSE_ITERATOR_H
#define MD_DENSE_ITERATOR_H

#include "../common/common.h"
#include "MDArray.h"
#include "MDIterator.h"
#include "MDCoord.h"
#include "Linearization.h"

/**
 * A class for iterating through a MDArray.  A MDDenseIterator visits
 * each element in the range it is responsible for once and only once,
 * in the order as defined by Linearization.
 *
 * Each MDDenseIterator has a Linearization object associated with it
 * to help define the moveNext() and movePrev() operations.  Suppose
 * the linearization associated with the iterator can be viewed as a
 * function \f$ f: N^n \to N \f$, and the iterator currently points to
 * coordinate x inside the n-dim array.  After a call to moveNext(),
 * the iterator will point to coordinate y s.t. \f$ f(y)=f(x)+1 \f$.
 * In other words, moveNext() moves the pointer to \f$ f^{-1}(f(x)+1)
 * \f$.  Similarly, movePrev() moves the pointer to \f$ f^{-1}(f(x)-1)
 * \f$.  Often, these operations can be accelerated by
 * Linearization::move() by providing a key difference of 1 or -1.
 *
 * \sa Iterator
 * \sa MDArray
 * \sa Linearization
 */

class MDDenseIterator: public MDIterator
{
public:
    /**
     * Constructs an iterator given the associated array and
     * linearization order.  Caller should make sure param
     * linearization is not shared with other iterators.
     *
     * \param array A pointer to the associated MDArray.
     *
     * \param linearization A pointer to a Linearization object that
     * defines the iterating order.
     */
    MDDenseIterator(MDArray *array, Linearization *linearization);

    /**
     * Constructs a copy of the given iterator.  Member
     * #linearization should be deep-copied.
     *
     * \param src Source to be copied.
     */
    MDDenseIterator(const MDDenseIterator &src);

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
     * The incremental computation ability of Linearization should be
     * used to avoid redundant computation whenever possible.
     *
     * \return true if successfully moved to the previous entry; false
     * if already before the beginning of the collection.
     *
     * \sa Linearization::move()
     */
    virtual bool movePrev();

    /**
     * Moves the iterator to the next entry in the collection.
     * "Next" is defined according to the linearization order.
     *
     * The incremental computation ability of Linearization should be
     * used to avoid redundant computation whenever possible.
     *
     * \return true if successfully moved to the next entry; false
     * if already after the end of the collection.
     *
     * \sa Linearization::move()
     */
    virtual bool moveNext();

    /**
     * Replaces the current entry with the specified datum.
     * 
     * \param  datum The datum to be put.
     */
    virtual void put(Datum_t &datum);

    /**
     * Virtual destructor.
     */
    virtual ~MDDenseIterator();

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
    /// Iteration begins at beginsAt (inclusive).
    MDCoord beginsAt;
    /// Iteration stops by endsBy (exclusive).
    MDCoord endsBy;

// private:
    MDCoord cursor;
};

#endif
