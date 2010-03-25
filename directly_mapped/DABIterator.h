#ifndef DAB_ITERATOR_H
#define DAB_ITERATOR_H

#include "../common/common.h"
#include "../common/ArrayInternalIterator.h"

class DABIterator : public ArrayInternalIterator
{

   private:
      Datum_t *data;
      Key_t beginsAt;
      Key_t endsBy;
      i64 size;
      i64 cur;

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
      bool setRange(const Key_t &b, const Key_t &e);
      bool setIndexRange(Key_t b, Key_t e);
};

#endif
