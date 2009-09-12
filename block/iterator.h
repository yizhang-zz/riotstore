#ifndef ITERATOR_H
#define ITERATOR_H

#include "common.h"

class Iterator {
	protected:
	Datum* current;

	public:
	virtual bool hasNext() =0;
	virtual Key getKey() =0;
	virtual Datum getDatum() =0;
};

class DenseBlockIterator: public Iterator {

	friend class DenseBlock;

	public:
	bool hasNext();
	Key getKey();	
	Datum getDatum();

	private:
	Key key;
	Datum* end;
	DenseBlockIterator(Range range, Datum* initial);
};

class EntryIterator: public Iterator {

   public:
   EntryIterator(unsigned numEntries, Entry* initial);

	public:
	bool hasNext();
	Key getKey();
	Datum getDatum();
   Entry getEntry();

   private:
   Entry* cur;
   Entry* end;
};
#endif
