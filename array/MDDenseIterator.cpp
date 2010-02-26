
#include "MDDenseIterator.h"

MDDenseIterator::MDDenseIterator(MDArray *array, Linearization *linearization)
{
   this->array = array;
   this->linearization = linearization->clone();
   if (array->getLinearization()->equals(this->linearization))
       accel = true;
   else
       accel = false;
   //beginsAt = linearization->unlinearize(0);
   //endsBy = linearization->unlinearize(array->size);
   beginIndex = 0;
   endIndex = array->size;
   reset();
}

MDDenseIterator::MDDenseIterator(const MDDenseIterator &src)
{
   array = src.array;
   linearization = src.linearization->clone();
   //beginsAt = src.beginsAt;
   //endsBy = src.endsBy;
   beginIndex = src.beginIndex;
   endIndex = src.endIndex;
   reset();
}

void MDDenseIterator::get(MDCoord &coord, Datum_t &datum)
{
    //coord = cursor;
    //array->get(cursor, datum);
    coord = linearization->unlinearize(cur);
    array->get(coord, datum);
}

bool MDDenseIterator::movePrev()
{
    //cursor = linearization->unlinearize(linearization->linearize(cursor) - 1);
    //return cursor != beginsAt;
    cur--;
    return cur != beginIndex;
}

bool MDDenseIterator::moveNext()
{
    //cursor = linearization->unlinearize(linearization->linearize(cursor) + 1);
    //return cursor != endsBy;
    cur++;
    return cur!=endIndex;
}

void MDDenseIterator::put(const Datum_t &datum)
{
    MDCoord d = linearization->unlinearize(cur);
    array->put(d, datum);
}

MDDenseIterator::~MDDenseIterator()
{
   delete linearization;
}

bool MDDenseIterator::setRange(const MDCoord &beginsAt, const MDCoord &endsBy)
{
    beginIndex = linearization->linearize(beginsAt);
    endIndex = linearization->linearize(endsBy);
    reset();
    return true;
}

bool MDDenseIterator::setIndexRange(Key_t begin, Key_t end)
{
    beginIndex = begin;
    endIndex = end;
    reset();
    return true;
}

void MDDenseIterator::reset()
{
    //cursor = linearization->move(beginsAt, -1);
    cur = beginIndex-1;
}
