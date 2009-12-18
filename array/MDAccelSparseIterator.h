#ifndef MD_ACCEL_SPARSE_ITERATOR_H
#define MD_ACCEL_SPARSE_ITERATOR_H

#include "MDSparseIterator.h"

/**
 * An accelerated version of MDSparseIterator.
 *
 * A major performance concern of MDSparseIterator is that most times
 * its operations can NOT be accelerated by
 * Linearization::unlinearizeIncremental().  This is because without
 * imposing any assumption on the array storage, one does not know
 * where the next or previous non-zero entry is.  Thus one cannot
 * assume a fixed increment of +1 or -1.  However, there is one
 * exception, namely when the linearization of the iterator coincides
 * with the linearization of the array's storage. In such a case, if
 * the array storage supports quick lookup of the next or previous
 * non-zero entry (e.g., a B-tree), then acceleration is possible:
 * coordinates need not be converted back and forth, and zero checking
 * of each element is avoided. 
 *
 * \sa MDSparseIterator
 * \sa MDArray
 * \sa Linearization
 */

class MDAccelSparseIterator: public MDSparseIterator
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
     * \parma itor An internal iterator for the array's storage.
     */
    MDAccelSparseIterator(MDArray *array, ArrayInternalIterator *itor);
    
    /**
     * Constructs a copy of the given iterator.  Base class' copy
     * constructor should be called in initialization list.
     *
     * \param src Source to be copied.
     */
    MDAccelSparseIterator(const MDAccelSparseIterator &src);

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
     * Replaces the current entry with the specified datum.
     * 
     * \param datum The datum to be put.
     */
    virtual void put(Datum_t &datum);

    /**
     * Virtual destructor.
     */
    virtual ~MDAccelSparseIterator();

    /**
     * \copydoc Iterator::setRange()
     */
    virtual bool setRange(MDCoord &beginsAt, MDCoord &endsBy);
    
protected:

    /// Iterator for the underlying storage of #array.
    ArrayInternalIterator *intIterator;

};

#endif
