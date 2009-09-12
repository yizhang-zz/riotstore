#ifndef ITERATOR_H
#define ITERATOR_H

#include "data.h"

class Iterator {
	protected:
	Data* current;

	public:
	virtual bool hasNext() =0;
	virtual Key getKey() =0;
	virtual Data getData() =0;
};

class DenseBlockIterator: public Iterator {

	friend class DenseBlock;

	public:
	bool hasNext();
	Key getKey();	
	Data getData();

	private:
	Key key;
	Data* end;
	DenseBlockIterator(Range range, Data* initial);
};

class EntryIterator: public Iterator {

   public:
   EntryIterator(unsigned numEntries, Entry* initial);

	public:
	bool hasNext();
	Key getKey();
	Data getData();
   Entry getEntry();

   private:
   Entry* cur;
   Entry* end;
};
#endif
