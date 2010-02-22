
#include "MDSparseIterator.h"

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
MDSparseIterator::MDSparseIterator(MDArray *array, Linearization *linearization)
{
   this->array = array;
   this->linearization = linearization->clone();
   beginsAt = MDCoord(linearization->unlinearize(-1));
   endsBy = beginsAt;
   cursor = beginsAt;
}
    
    /**
     * Constructs a copy of the given iterator.
     *
     * \param src Source to be copied.
     */
MDSparseIterator::MDSparseIterator(const MDSparseIterator &src)
{
   this->array = src.array;
   this->linearization = src.linearization->clone();
   beginsAt = src.beginsAt;
   endsBy = src.endsBy;
   cursor = src.cursor;
}

    /**
     * Gets the current coordinate and the datum the iterator points to.
     *
     * \param [out] coord The current coordinate.
     * \param [out] datum The current datum.
     */
    void MDSparseIterator::get(MDCoord &coord, Datum_t &datum)
{
   coord = cursor;
   array->get(cursor, datum);
}

    /**
     * Moves the iterator to the previous entry in the collection.
     * "Previous" is defined according to the linearization order.
     *
     * \return true if successfully moved to the previous entry; false
     * if already before the beginning of the collection.
     *
     */
    bool MDSparseIterator::movePrev()
{
   do
   {
      cursor = linearization->unlinearize(linearization->linearize(cursor) - 1);
      Datum_t datum;
      array->get(cursor, datum);
   } while (datum == 0);
   return cursor == beginsAt;
}

    /**
     * Moves the iterator to the next entry in the collection.
     * "Next" is defined according to the linearization order.
     *
     * \return true if successfully moved to the next entry; false
     * if already after the end of the collection.
     *
     */
    bool MDSparseIterator::moveNext()
{
   do
   {
      cursor = linearization->unlinearize(linearization->linearize(cursor) + 1);
      Datum_t datum;
      array->get(cursor, datum);
   } while (datum == 0);
   return cursor == endsBy;
}

    /**
     * Replaces the current entry with the specified datum.
     * 
     * \param [out] datum The datum to be put.
     */
    void MDSparseIterator::put(Datum_t &datum)
{
   array->put(cursor, datum);
}

    /**
     * Virtual destructor.
     */
    MDSparseIterator::~MDSparseIterator()
{
   delete linearization;
}

    /**
     * \copydoc Iterator::setRange()
     */
    bool MDSparseIterator::setRange(MDCoord &beginsAt, MDCoord &endsBy)
{
   this->beginsAt = beginsAt;
   this->endsBy = endsBy;
}
   
void MDSparseIterator::reset()
{
   cursor = beginsAt;
}
