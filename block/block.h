#ifndef BLOCK_H
#define BLOCK_H

#include "common.h"
#include "iterator.h"
#include "SearchResult.h"

#include <cstdio>

enum ReturnStatus { NORMAL, OUT_OF_RANGE, OVERFLOW, IMPROPER_RANGE };

class Block {
	protected:
	BlockHeader* header;
	void* payload;	/* pointer to payload struct, payload implementations in derived classes */
	/* NOTE: we do not actually store data in the payload_ptr. Instead, we just
	point it to the right offset in the (memory-mapped) disk block. So after a
	fread() call, which reads a disk block into memory, we pass the address of 
	the memory block to the constructor of Block. The payload_ptr pointer
	is then initialized to point to the payload section. No data copying is 
	done. */

	public:
	/* Factory method for creating a proper subclass of Block given
	a pointer to a memory-mapped disk block. */
	static Block* createBlock(void* block);
	/* pseudo code: switch(type) {case BLK_DENSE: return new DenseBlock(...);...} */
	static long readBlock(FILE* file, void* target);

	BlockType getType(); // leaf, internal, or root?
	BlockFormat getFormat(); // dense or sparse
	
	uint32_t getEntryCount();
	Datum getDefaultValue();
	
	Block();
	Block(int capacity, BlockType type);
	virtual ~Block();
	
	static int getBoundary(); // split point must be multiples of boundary
	
	void setRange(Key lower, Key upper);
	Range getRange();
	Key getLowerBound();
	Key getUpperBound();
	void setLowerBound(Key lower);
	void setUpperBound(Key upper);
	
	/* recursively search for an entry with key at leaf level */
	SearchResult search(Key key);
	/* searches for key in current block. return the index if found,
	otherwise the index of key if it is to be inserted. */
	int locate(Key key);
	
	/* Insert a child pointer to the current block; key is the lower bound key of child. */
	virtual int put(BlockNo child, Key key);
	
	virtual int get(Key key, Datum& value) =0;
	virtual Iterator* getIterator(Range range) =0;
	virtual int put(Key key, Datum value) =0; 
	virtual int put(Iterator* iterator) =0;	
	/* del functions equivalent to put(key, DEFAULT_VALUE) */
	virtual int del(Key key) =0;
	virtual int del(Range range) =0;
	virtual long write(FILE* file) =0;
};

class DenseBlock : public Block {
	private:
	Datum* data; 

	public:
	DenseBlock(void* block);
	DenseBlock(BlockHeader* blockheader, Datum* entries);
    DenseBlock(Range r, unsigned nextBlock, Datum def=0);
    ~DenseBlock();

	public:
	int get(Key key, Datum& value);
	Iterator* getIterator(Range range);
	int put(Key key, Datum value); 
	int put(Iterator* iterator);
	int put(BlockNo child, Key key);
	
	/* del functions equivalent to put(key, DEFAULT_VALUE) */
	int del(Key key);
	int del(Range range);
	long write(FILE* file);

	private:
	inline bool inRange(Key key);
	inline Datum* getDatum(Key key);
	inline bool isNewDatum(Datum* target, Datum replacement);
	inline bool isDelDatum(Datum* target, Datum replacement);
};

/* Prefix compression seems not urgent right now... */
class SparseBlock : public Block {
	private:
	/* SparseHeader s_header;*/
	Datum* data;
	char* key; /* variable-size keys */

	public:
	SparseBlock(void* block);

	unsigned getMaxNumEntries();

	int get(Key key, Datum& value);
	Iterator* getIterator(Range range);
	int put(Key key, Datum value); 
	int put(Iterator* iterator);
	int put(BlockNo child, Key key);
	
	/* del functions equivalent to put(key, DEFAULT_VALUE) */
	int del(Key key);
	int del(Range range);
	long write(FILE* file);
};

#endif
