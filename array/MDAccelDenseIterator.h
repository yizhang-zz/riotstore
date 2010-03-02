#ifndef MD_ACCEL_DENSE_ITERATOR_H
#define MD_ACCEL_DENSE_ITERATOR_H

#include "MDDenseIterator.h"

/**
 * An accelerated version of MDDenseIterator.
 *
 * A major performance concern of MDDenseIterator is that an operatoin
 * such as MDDenseIterator::moveNext involves two (un)linearization
 * operations.  First, the current coordinate is linearized and
 * incremented and unlinearized back.  Second, in order to retrieve
 * the data, the new coordinate is passed to the array and linearized
 * to an index in the 1-D storage space.  This is inevitable if we
 * make no assumption about the linearization functions used by the
 * storage and the iterator.  However, there is one exception, namely
 * when the linearization of the iterator coincides with the
 * linearization of the array's storage.  In such a case, if the array
 * storage supports iteration in its 1-D space (and it should), then
 * acceleration is possible. This class provides such acceleration.
 *
 * \sa MDDenseIterator
 * \sa MDArray
 * \sa Linearization
 */

class MDAccelDenseIterator: public MDDenseIterator
{
public:

    /**
     * Constructs an iterator given the associated array.  Array's
     * storage linearization is used as the iterator's linearization.
     * As an optimization, moveNext() or movePrev() does not have to
     * scan the entries to find the next or previous non-zero entry.
     * Instead, #intIterator is used to directly iterate through the
     * array's storage.
     *
     * Make sure to keep a copy of array's linearization object.
     *
     * \param array The associated MDArray.
     * \param itor An internal iterator for the array's storage.
     */
    MDAccelDenseIterator(MDArray *array, ArrayInternalIterator *itor);
    
    /**
     * Constructs a copy of the given iterator.  Base class' copy
     * constructor should be called in initialization list.
     *
     * \param src Source to be copied.
     */
    MDAccelDenseIterator(const MDAccelDenseIterator &src);

    /**
     * Moves the iterator to the previous entry in the collection.
     * "Previous" is defined according to the linearization order.
     * #intIterator's ArrayInternalIterator::movePrev() should be
     * called.
     *
     * \return true if successfully moved to the previous entry; false
     * if already before the beginning of the collection.
     *
     */
    virtual bool movePrev();

    /**
     * Moves the iterator to the next entry in the collection.  "Next"
     * is defined according to the linearization order.
     * #intIterator's ArrayInternalIterator::moveNext() should be
     * called.
     *
     * \return true if successfully moved to the next entry; false
     * if already after the end of the collection.
     *
     */
    virtual bool moveNext();

    /**
     * Gets the current coordinate and the datum the iterator points to.
     *
     * \param [out] coord The current coordinate.
     * \param [out] datum The current datum.
     */
    virtual void get(MDCoord &coord, Datum_t &datum);

    /**
     * Replaces the current entry with the specified datum.
     * 
     * \param datum The datum to be put.
     */
    virtual void put(Datum_t &datum);

    /**
     * Virtual destructor.
     */
    virtual ~MDAccelDenseIterator();

    /**
     * \copydoc Iterator::setRange()
     */
    virtual bool setRange(MDCoord &beginsAt, MDCoord &endsBy);

    virtual void reset();
    
protected:

    /// Iterator for the underlying storage of #array.
    ArrayInternalIterator *intIterator;

};

#endif
