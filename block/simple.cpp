#include "simple.h"
#include <cstdlib>
using namespace std;

SimpleManager::SimpleManager(const char* name, Data def)
{
   fileName = name;
   defaultValue = def;
   setRange(range, 0, -1);
   nBlocks = 0;
   buffer = malloc(BUFFER_SIZE);
   file = fopen(fileName, MODE_W);
   addBlock();
   fclose(file);
}

long SimpleManager::addBlock()
{
   Range r;
   setRange(r, range.upperBound + 1, range.upperBound + NUM_DENSE_ENTRIES);
   DataBlock* block = new DenseBlock(r, ++nBlocks, defaultValue);
   long size = block->write(file);
   delete block;
   range.upperBound += NUM_DENSE_ENTRIES;
   nBlocks++;
   return size;
}

inline bool SimpleManager::inRange(Key key)
{
   return (range.lowerBound <= key && key <= range.upperBound);
}

long SimpleManager::loadBlock(DataBlock* block, long offset)
{
   fseek(file, offset*BLOCK_SIZE, SEEK_SET);
   long size = DataBlock::readBlock(file, buffer);
   block = DataBlock::createBlock(buffer);
   return size;
}

long SimpleManager::writeBlock(DataBlock* block, long offset)
{
   fseek(file, offset*BLOCK_SIZE, SEEK_SET);
   return block->write(file);
}

int SimpleManager::get(Key key, Data& value)
{
   if (inRange(key))
   {
      DataBlock* block;
      file = fopen(fileName, MODE_R);
      loadBlock(block, OFFSET(key));
      fclose(file);
      return block->get(key, value);
   }
   return OUT_OF_RANGE;
}

Iterator* SimpleManager::getIterator(Range range)
{
   
}

int SimpleManager::put(Key key, Data value)
{
   while(!inRange(key))
   {
      addBlock();
   }
   DataBlock* block;
   file = fopen(fileName, MODE_U);
   loadBlock(block, OFFSET(key));
   fflush(file);
   int status = block->put(key, value);
   writeBlock(block, OFFSET(key));
   fclose(file);
   return status;
}

int SimpleManager::put(Iterator* iterator)
{

}

int main() {return 0;}
