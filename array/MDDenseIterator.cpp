
#include "MDDenseIterator.h"

MDDenseIterator::MDDenseIterator(MDArray *array, Linearization *linearization)
{
   this->array = array;
   this->linearization = linearization->clone();
   beginsAt = MDCoord(linearization->unlinearize(-1));
   endsBy = MDCoord(beginsAt);
   cursor = MDCoord(beginsAt);
}

MDDenseIterator::MDDenseIterator(const MDDenseIterator &src)
{
   array = src.array;
   linearization = src.linearization->clone();
   beginsAt = src.beginsAt;
   endsBy = src.endsBy;
   cursor = src.cursor;
}

void MDDenseIterator::get(MDCoord &coord, Datum_t &datum)
{
   coord = cursor;
   array->get(cursor, datum);
}

bool MDDenseIterator::movePrev()
{
   cursor = linearization->unlinearize(linearization->linearize(cursor) - 1);
   return cursor == beginsAt;
}

bool MDDenseIterator::moveNext()
{
   cursor = linearization->unlinearize(linearization->linearize(cursor) + 1);
   return cursor == endsBy;
}

void MDDenseIterator::put(Datum_t &datum)
{
   array->put(cursor, datum);
}

MDDenseIterator::~MDDenseIterator()
{
   delete linearization;
}

bool MDDenseIterator::setRange(MDCoord &beginsAt, MDCoord &endsBy)
{
   this->beginsAt = beginsAt;
   this->endsBy = endsBy;
}

void MDDenseIterator::reset()
{
   cursor = beginsAt;
}
