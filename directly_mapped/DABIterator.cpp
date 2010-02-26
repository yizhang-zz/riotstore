
#include "DABIterator.h"

DABIterator::DABIterator(Datum_t *_cur, Key_t _beginsAt, Key_t _endsBy)
{
    data = _cur;
    beginsAt = _beginsAt;
    endsBy = _endsBy;
    reset();
}

/*
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
*/

DABIterator::~DABIterator() {}

/// checks if next element exists by incrementing cur, then comparing with
/// end
bool DABIterator::moveNext() 
{
   cur++;
   return cur < endsBy;
}

bool DABIterator::movePrev()
{
   cur--;
   return cur >= beginsAt;
}

void DABIterator::get(Key_t &k, Datum_t &d)
{
    k = cur+beginsAt;
    d = data[cur];
}

void DABIterator::put(const Datum_t &d)
{
    data[cur] = d;
}

void DABIterator::reset()
{
   cur = beginsAt-1;
}

bool DABIterator::setIndexRange(Key_t b, Key_t e)
{
    beginsAt = b;
    endsBy = e;
    reset();
    return true;
}

bool DABIterator::setRange(const Key_t &b, const Key_t &e)
{
    throw("setRange of DABIterator not implemented");
    return false;
}
