#ifndef SIMPLE_H
#define SIMPLE_H

#include "data.h"
#include "iterator.h"
#include "block.h"
#include <cstdio>

#define BUFFER_SIZE 4096 

#define MODE_R "rb"
#define MODE_W "wb"
#define MODE_A "ab"
#define MODE_U "rb+"
#define MODE_RW "wb+"
#define MODE_RA "ab+"

#define OFFSET(key) (key / NUM_DENSE_ENTRIES)

class SimpleManager
{
   private:
      const char* fileName;
      Data defaultValue;
      Range range;
      unsigned nBlocks;
      void* buffer;
      FILE* file;

   public:
      SimpleManager(const char* name, Data def = 0);

	private:
      long addBlock();
      inline bool inRange(Key key);
      long loadBlock(DataBlock* block, long offset);
      long writeBlock(DataBlock* block, long offset);

	public:
      int get(Key key, Data& value);
      Iterator* getIterator(Range range);
      int put(Key key, Data value); 
      int put(Iterator* iterator);	
};

#endif
