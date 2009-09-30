#ifndef SIMPLE_H
#define SIMPLE_H

#include "../common/common.h"
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

#define OFFSET(key) (key / CAPACITY_DENSE)

class SimpleManager
{
   private:
      const char* fileName;
      Datum defaultValue;
      Range range;
      unsigned nBlocks;
      void* buffer;
      FILE* file;

   public:
      SimpleManager(const char* name, Datum def = 0);

	private:
      long addBlock();
      inline bool inRange(Key key);
      long loadBlock(Block* block, long offset);
      long writeBlock(Block* block, long offset);

	public:
      int get(Key key, Datum& value);
      Iterator* getIterator(Range range);
      int put(Key key, Datum value); 
      int put(Iterator* iterator);	
};

#endif
