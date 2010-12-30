// -*- mode: c++ -*-
#ifndef BLOCK_H
#define BLOCK_H

#include "../common/common.h"

enum ReturnStatus { RS_NORMAL, RS_OUT_OF_RANGE, RS_OVERFLOW, RS_IMPROPER_RANGE };

template<class _K, class _D>
class Block {
	
protected:

	// [lowerBound, upperBound) specifies the key range of this block
	_K lowerBound;
	_K upperBound;
	PageHandle ph; // contains PID and a pointer to the in-memory image
	
public:

    // TODO: implement these?
	Block() {}
	virtual ~Block() {}
    PageHandle getPageHandle() {return ph;}
	
	// Factory method for creating a proper subclass of Block.
	static Block<_K, _D>* initFromPage(PageHandle *ph, _K begin, _K end, _D
            defaultValue) {}
	virtual void flush() {}

    // TODO: implement these?
	//BlockType getType() const {} // leaf or internal
	//BlockFormat getFormat() const {} // dense or sparse
	//_D getDefaultValue() const {}

	void setRange(_K lower, _K upper) {
        lowerBound = lower;
        upperBound = upper;
    }

	_K getLowerBound() const {
        return lowerBound;
    }

	_K getUpperBound() const {
        return upperBound;
    }
	
    void setLowerBound(_K lower) {
        lowerBound = lower;
    }

	void setUpperBound(_K upper) {
        upperBound = upper;
    }
	
/*
	PID_t getPID() const {
		return ph.pid;
	}
*/
	//virtual Iterator* getIterator(Range range) = 0;
};

/*
class DenseBlock : public Block {
	private:
	Datum_t* data; 

	public:
	DenseBlock(void* block);
	DenseBlock(BlockHeader* blockheader, Datum_t* entries);
    DenseBlock(Range r, unsigned nextBlock, Datum_t def=0);
    ~DenseBlock();

	public:
	int get(Key_t key, Datum_t& value);
	Iterator* getIterator(Range range);
	int put(Key_t key, Datum_t value); 
	int put(Iterator* iterator);
	int put(BlockNo child, Key_t key);
	
	// del functions equivalent to put(key, DEFAULT_VALUE) 
	int del(Key_t key);
	int del(Range range);
	//long write(FILE* file);

	private:
	inline bool inRange(Key_t key);
	inline Datum_t* getDatum_t(Key_t key);
	inline bool isNewDatum_t(Datum_t* target, Datum_t replacement);
	inline bool isDelDatum_t(Datum_t* target, Datum_t replacement);
};

*/

/* Prefix compression seems not urgent right now... */
/*
class SparseBlock : public Block {
	private:
	// SparseHeader s_header;
	Datum_t* data;
	char* key; // variable-size keys 

	public:
	SparseBlock(void* block);

	unsigned getMaxNumEntries();

	int get(Key_t key, Datum_t& value);
	Iterator* getIterator(Range range);
	int put(Key_t key, Datum_t value); 
	int put(Iterator* iterator);
	int put(BlockNo child, Key_t key);
	
	// del functions equivalent to put(key, DEFAULT_VALUE) 
	int del(Key_t key);
	int del(Range range);
	//long write(FILE* file);
};
*/
#endif
