
#include "MDAccelDenseIterator.h"


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
   MDAccelDenseIterator::MDAccelDenseIterator(MDArray *array,
         ArrayInternalIterator *itor) : MDDenseIterator::MDDenseIterator(array,
            array->getLinearization())
   {
   /*   this->array = array;
      this->linearization = array->getLinearization();*/
      intIterator = itor;
     /* beginsAt = MDCoord(linearization->unlinearize(0));
      endsBy = beginsAt;
      cursor = beginsAt;*/
   }
    
    /**
     * Constructs a copy of the given iterator.  Base class' copy
     * constructor should be called in initialization list.
     *
     * \param src Source to be copied.
     */
    MDAccelDenseIterator::MDAccelDenseIterator(const MDAccelDenseIterator &src)
   : MDDenseIterator::MDDenseIterator(src)
    {
      intIterator = src.intIterator;
    }

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
    bool MDAccelDenseIterator::movePrev()
    {
       cursor = linearization->move(cursor, -1);
       intIterator->movePrev();
       return cursor == linearization->move(beginsAt, -1);
    }

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
    bool MDAccelDenseIterator::moveNext()
    {
       cursor = linearization->move(cursor, 1);
       intIterator->moveNext();
       return cursor == endsBy;
    }

    /**
     * Gets the current coordinate and the datum the iterator points to.
     *
     * \param [out] coord The current coordinate.
     * \param [out] datum The current datum.
     */
    void MDAccelDenseIterator::get(MDCoord &coord, Datum_t &datum)
    {
       coord = cursor;
       Key_t key;
       intIterator->get(key, datum);
    }

    /**
     * Replaces the current entry with the specified datum.
     * 
     * \param datum The datum to be put.
     */
    void MDAccelDenseIterator::put(Datum_t &datum)
    {
       intIterator->put(datum);
    }

    /**
     * Virtual destructor.
     */
    MDAccelDenseIterator::~MDAccelDenseIterator()
    {
       // delete linearization;
    }

    /**
     * \copydoc Iterator::setRange()
     */
    bool MDAccelDenseIterator::setRange(MDCoord &beginsAt, MDCoord &endsBy)
    {
       this->beginsAt = beginsAt;
       this->endsBy = endsBy;
       return linearization->linearize(beginsAt) >= linearization->linearize(endsBy);
    }
    
void MDAccelDenseIterator::reset()
{
   cursor = beginsAt;
}
