
#include "DABIterator.h"

DABIterator::DABIterator(Datum_t *_cur, Key_t _beginsAt, Key_t _endsBy)
{
   beginsAt = _beginsAt;
   endsBy = _endsBy;
   cur = _cur-1;
   begin = cur;
   end = _cur + (_endsBy - _beginsAt);
}

// need to make deep copy?
// still need this?
DABIterator::DABIterator(const DABIterator& obj) 
{
   beginsAt = obj.beginsAt;
   endsBy = obj.endsBy;
   cur = obj.cur;
   begin = cur;
   end = obj.end;
}

DABIterator::~DABIterator() {}

/// checks if next element exists by incrementing cur, then comparing with
/// end
bool DABIterator::moveNext() 
{
   cur++;
   return cur != end;
}

bool DABIterator::movePrev()
{
   cur--;
   return cur != begin;
}

void DABIterator::get(Key_t &k, Datum_t &d)
{
   if (cur == begin)
   {
      k = NA_INT;
      d = NA_DOUBLE;
   }
   else
   {
      k = beginsAt + cur - begin - 1;
      d = *cur;
   }
}

void DABIterator::put(const Datum_t &d)
{
   if (cur != begin)
      *cur = d;
}

void DABIterator::reset()
{
   cur = begin;
}
