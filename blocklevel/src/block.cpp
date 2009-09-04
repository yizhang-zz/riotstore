#include "block.h"
using namespace std;

DataBlock* DataBlock::createBlock(void* block) 
{
	/* assumes block type MUST be first element of each block */
	int* type = (int*) block;
	switch(*type){
		case BLOCK_DENSE:
			return new DenseBlock(block);
		case BLOCK_SPARSE:
			return new SparseBlock(block);
		default:
			return NULL;
	}
}

long DataBlock::readBlock(FILE* file, void* target) 
{
	long start = ftell(file);
	fread(target, BLOCK_SIZE, 1, file);
	long stop = ftell(file);
	return stop - start;
	/*
	fread(header, sizeof(BlockHeader), 1, file);
	fread(data, sizeof(Data), NUM_DENSE_ENTRIES, file);
	cout<<ftell(file)<<endl;
	return 0;
	*/
}

Range DataBlock::getRange() 
{
	return header->range;
}

int DataBlock::getType() 
{
	return header->type;
}

unsigned DataBlock::getNumEntries() 
{
	return header->nEntries;
}
	
Data DataBlock::getDefaultValue() 
{
	return header->defaultValue;
}

DenseBlock::DenseBlock(void* block) 
{
	/*
	ptr references memory location of block, [header|payload]
	header_ptr = ptr
	payload_ptr = header_ptr + sizeof header
	data_ptr = payload_ptr
	initialize BlockHeader
	~~ initialize data[] to default value? ~~
	*/

	header = (BlockHeader*) block;
	data = (Data*) (header+1);
	payload = data;
}

DenseBlock::DenseBlock(BlockHeader* blockheader, Data* entries) 
{
	header = blockheader;
	data = entries;
	payload = data;
}

DenseBlock::DenseBlock(Range r, unsigned nextBlock, Data def)
{
   header = new BlockHeader;
   setBlockHeader(header, BLOCK_DENSE, r, nextBlock, def);
   data = new Data[NUM_DENSE_ENTRIES];
   for(int k=0; k<NUM_DENSE_ENTRIES; k++)
   {
      *(data+k) = R_ValueOfNA();
   }
   payload = data;
}

DenseBlock::~DenseBlock()
{
   /* only needed explicited when DenseBlock(Range, unsigned, Data)
      constructor is invoked
      */
   delete header;
   delete[] data;
}

inline bool DenseBlock::inRange(Key key) 
{
	return ((header->range.lowerBound)<=key && key<=(header->range.upperBound));
}

inline Data* DenseBlock::getData(Key key) 
{
	return data + (key - header->range.lowerBound);
}

int DenseBlock::get(Key key, Data& value)
{
	if(inRange(key)) 
   {
		value = *(getData(key));
		return NORMAL;
	}
	return OUT_OF_RANGE;
}

Iterator* DenseBlock::getIterator(Range range)
{
	return new DenseBlockIterator(range, getData(range.lowerBound));
}

inline bool DenseBlock::isNewData(Data* target, Data replacement) 
{
   return (R_IsNA(*target) && !R_IsNA(replacement));
}

inline bool DenseBlock::isDelData(Data* target, Data replacement) 
{
   return (!R_IsNA(*target) && R_IsNA(replacement));
}

int DenseBlock::put(Key key, Data value) 
{
	if(inRange(key)) {
		Data* target = getData(key);
		if(isNewData(target, value)) {
			(header->nEntries)++;
		}
		else if(isDelData(target, value)) {
			(header->nEntries)--;
		}
	
		*target = value;
		return NORMAL;
	}
	return OVERFLOW;
}

int DenseBlock::put(Iterator* iterator) 
{  
	/*
      returns pointer to array of entries who did not fit in block
	*/
   int status = NORMAL;
   while(iterator->hasNext()) {
      status = put(iterator->getKey(), iterator->getData());
      if(status == OVERFLOW)
         return OVERFLOW;
   }
	return NORMAL;
}

int DenseBlock::del(Key key) 
{
	return put(key, header->defaultValue);
}

int DenseBlock::del(Range range) 
{
}

long DenseBlock::write(FILE* file) 
{
	/**
	write_header:
		write each member
		fseek to sizeof(Header)
		write dense payload: fwrite(data_ptr, 8, size of range, to file)
	**/
	
	long start = ftell(file); 
	fwrite(header, sizeof(BlockHeader), 1, file);
	fwrite(data, sizeof(Data), NUM_DENSE_ENTRIES, file);
	long stop = ftell(file);
	return stop-start;
}


SparseBlock::SparseBlock(void* block)
{
}

int SparseBlock::get(Key key, Data& value)
{
}

Iterator* SparseBlock::getIterator(Range range)
{
}

int SparseBlock::put(Key key, Data value)
{
}

int SparseBlock::put(Iterator* iterator)
{
}

int SparseBlock::del(Key key)
{
}

int SparseBlock::del(Range range)
{
}

long SparseBlock::write(FILE* file) 
{
}



