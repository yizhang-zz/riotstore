#ifndef DAB_ITERATOR_H
#define DAB_ITERATOR_H

#include "../common/common.h"
#include "../common/ArrayInternalIterator.h"

class DABIterator : public ArrayInternalIterator
{

   private:
      Datum_t *cur;
      Datum_t *begin;
      Datum_t *end;

   public:
      DABIterator(Datum_t *_cur, Key_t _beginsAt, Key_t _endsBy);
      // need to make deep copy?
      // still need this?
      DABIterator(const DABIterator& obj);
      ~DABIterator();

      bool moveNext();
      bool movePrev();
      void get(Key_t &k, Datum_t &d);
      void put(const Datum_t &d);
      void reset();
};

#endif
