#include <cstdlib>
#include "simple.h"
using namespace std;

SimpleManager::SimpleManager(const char* name, Datum def)
{
   fileName = name;
   defaultValue = def;
   SetRange(range, 0, -1);
   nBlocks = 0;
   buffer = malloc(BUFFER_SIZE);
   file = fopen(fileName, MODE_W);
   addBlock();
   fclose(file);
}

long SimpleManager::addBlock()
{
   Range r;
   SetRange(r, range.upperBound + 1, range.upperBound + CAPACITY_DENSE);
   Block* block = new DenseBlock(r, ++nBlocks, defaultValue);
   long size = block->write(file);
   delete block;
   range.upperBound += CAPACITY_DENSE;
   nBlocks++;
   return size;
}

inline bool SimpleManager::inRange(Key key)
{
   return (range.lowerBound <= key && key <= range.upperBound);
}

long SimpleManager::loadBlock(Block* block, long offset)
{
   fseek(file, offset*BLOCK_SIZE, SEEK_SET);
   long size = Block::readBlock(file, buffer);
   block = Block::createBlock(buffer);
   return size;
}

long SimpleManager::writeBlock(Block* block, long offset)
{
   fseek(file, offset*BLOCK_SIZE, SEEK_SET);
   return block->write(file);
}

int SimpleManager::get(Key key, Datum& value)
{
   if (inRange(key))
   {
      Block* block;
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

int SimpleManager::put(Key key, Datum value)
{
   while(!inRange(key))
   {
      addBlock();
   }
   Block* block;
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
