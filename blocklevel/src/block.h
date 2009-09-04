#ifndef BLOCK_H
#define BLOCK_H

#include "data.h"
#include "iterator.h"
#include <cstdio>

enum ReturnStatus { NORMAL, OUT_OF_RANGE, OVERFLOW, IMPROPER_RANGE };

class DataBlock {
	protected:
	BlockHeader* header;
	void* payload;	/* pointer to payload struct, payload implementations in derived classes */
	/* NOTE: we do not actually store data in the payload_ptr. Instead, we just
	point it to the right offset in the (memory-mapped) disk block. So after a
	fread() call, which reads a disk block into memory, we pass the address of 
	the memory block to the constructor of DataBlock. The payload_ptr pointer
	is then initialized to point to the payload section. No data copying is 
	done. */

	public:
	/* Factory method for creating a proper subclass of DataBlock given
	a pointer to a memory-mapped disk block. */
	static DataBlock* createBlock(void* block);
	/* pseudo code: switch(type) {case BLK_DENSE: return new DenseBlock(...);...} */
	static long readBlock(FILE* file, void* target);

	int getType();
	unsigned getNumEntries();
	Range getRange();
	Data getDefaultValue();
	
	virtual int get(Key key, Data& value) =0;
	virtual Iterator* getIterator(Range range) =0;
	virtual int put(Key key, Data value) =0; 
	virtual int put(Iterator* iterator) =0;	
	/* del functions equivalent to put(key, DEFAULT_VALUE) */
	virtual int del(Key key) =0;
	virtual int del(Range range) =0;
	virtual long write(FILE* file) =0;
};

class DenseBlock : public DataBlock {
	private:
	Data* data; 

	public:
	DenseBlock(void* block);
	DenseBlock(BlockHeader* blockheader, Data* entries);
   DenseBlock(Range r, unsigned nextBlock, Data def=0);
   ~DenseBlock();

	public:
	int get(Key key, Data& value);
	Iterator* getIterator(Range range);
	int put(Key key, Data value); 
	int put(Iterator* iterator);	
	/* del functions equivalent to put(key, DEFAULT_VALUE) */
	int del(Key key);
	int del(Range range);
	long write(FILE* file);

	private:
	inline bool inRange(Key key);
	inline Data* getData(Key key);
	inline bool isNewData(Data* target, Data replacement);
	inline bool isDelData(Data* target, Data replacement);
};

class SparseBlock : public DataBlock {
	private:
	SparseHeader s_header;
	Data* data;
	char* key; /* variable-size keys */

	public:
	SparseBlock(void* block);

	unsigned getMaxNumEntries();

	int get(Key key, Data& value);
	Iterator* getIterator(Range range);
	int put(Key key, Data value); 
	int put(Iterator* iterator);
	/* del functions equivalent to put(key, DEFAULT_VALUE) */
	int del(Key key);
	int del(Range range);
	long write(FILE* file);
};

#endif
