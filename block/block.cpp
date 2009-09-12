#include "block.h"
using namespace std;

Block* Block::createBlock(void* block) 
{
	/* assumes block type MUST be first element of each block */
	BlockType* type = (BlockType*) block;
	switch(*type){
		case DENSE:
			return new DenseBlock(block);
		case SPARSE:
			return new SparseBlock(block);
		default:
			return NULL;
	}
}

long Block::readBlock(FILE* file, void* target) 
{
	long start = ftell(file);
	fread(target, BLOCK_SIZE, 1, file);
	long stop = ftell(file);
	return stop - start;
	/*
	fread(header, sizeof(BlockHeader), 1, file);
	fread(data, sizeof(Datum), NUM_DENSE_ENTRIES, file);
	cout<<ftell(file)<<endl;
	return 0;
	*/
}

Range Block::getRange() 
{
	return header->range;
}

BlockType Block::getType() 
{
	return header->type;
}

uint32_t Block::getEntryCount() 
{
	return header->entry_count;
}
	
Datum Block::getDefaultValue() 
{
	return header->default_value;
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
	data = (Datum*) (header+1);
	payload = data;
}

DenseBlock::DenseBlock(BlockHeader* blockheader, Datum* entries) 
{
	header = blockheader;
	data = entries;
	payload = data;
}

DenseBlock::DenseBlock(Range r, unsigned nextBlock, Datum def)
{
   header = new BlockHeader;
   SetBlockHeader(header, DENSE, r, nextBlock, def);
   data = new Datum[CAPACITY_DENSE];
   for(int k=0; k<CAPACITY_DENSE; k++)
   {
      *(data+k) = R_ValueOfNA();
   }
   payload = data;
}

DenseBlock::~DenseBlock()
{
   /* only needed explicited when DenseBlock(Range, unsigned, Datum)
      constructor is invoked
      */
   delete header;
   delete[] data;
}

inline bool DenseBlock::inRange(Key key) 
{
	return ((header->range.lowerBound)<=key && key<=(header->range.upperBound));
}

inline Datum* DenseBlock::getDatum(Key key) 
{
	return data + (key - header->range.lowerBound);
}

int DenseBlock::get(Key key, Datum& value)
{
	if(inRange(key)) 
   {
		value = *(getDatum(key));
		return NORMAL;
	}
	return OUT_OF_RANGE;
}

Iterator* DenseBlock::getIterator(Range range)
{
	return new DenseBlockIterator(range, getDatum(range.lowerBound));
}

inline bool DenseBlock::isNewDatum(Datum* target, Datum replacement) 
{
   return (R_IsNA(*target) && !R_IsNA(replacement));
}

inline bool DenseBlock::isDelDatum(Datum* target, Datum replacement) 
{
   return (!R_IsNA(*target) && R_IsNA(replacement));
}

int DenseBlock::put(Key key, Datum value) 
{
	if(inRange(key)) {
		Datum* target = getDatum(key);
		if(isNewDatum(target, value)) {
			(header->entry_count)++;
		}
		else if(isDelDatum(target, value)) {
			(header->entry_count)--;
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
      status = put(iterator->getKey(), iterator->getDatum());
      if(status == OVERFLOW)
         return OVERFLOW;
   }
	return NORMAL;
}

int DenseBlock::del(Key key) 
{
	return put(key, header->default_value);
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
	fwrite(data, sizeof(Datum), CAPACITY_DENSE, file);
	long stop = ftell(file);
	return stop-start;
}


SparseBlock::SparseBlock(void* block)
{
}

int SparseBlock::get(Key key, Datum& value)
{
}

Iterator* SparseBlock::getIterator(Range range)
{
}

int SparseBlock::put(Key key, Datum value)
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



