#include "../block.h"
#include <iostream>
#include <cstdlib>
#include <cassert>
using namespace std;

Range r;
Range* rp = &r;
BlockHeader* bhp = new BlockHeader;
Data* dp = new Data[NUM_DENSE_ENTRIES];

void testPutGet(DataBlock* dbp, const char* className) 
{
	Data datum = -1;
	assert(dbp->put(5, 10)==NORMAL);
	assert(dbp->get(5, datum)==NORMAL && datum==10);
	assert(dbp->put(-5, 10)==OVERFLOW);
	assert(dbp->get(-5, datum)==OUT_OF_RANGE);
	assert(dbp->put(0, 12)==NORMAL);
	assert(dbp->get(0, datum)==NORMAL && datum==12);
	cout << className << "::put(Key, Data) test passed" << endl;
	cout << className << "::get(Key, Data&) test passed" << endl;
}

void testIterator(DataBlock* dbp, const char* className) 
{
	for(int k=0; k<NUM_DENSE_ENTRIES; k++){
		dbp->put(k, k+1);
	}
	Iterator* i = dbp->getIterator(r);
	int count = 0;
	while(i->hasNext()){
		assert(i->getKey()==(count++));
		assert(i->getData()==count);
	}
   delete i;
	cout << className <<"::getKey() test passed" << endl;
	cout << className <<"::getData() test passed" << endl;
	assert(count==NUM_DENSE_ENTRIES);
	cout << className <<"::hasNext()  test passed" << endl;
}

void testWrite(DataBlock* dbp, FILE* file, const char* className) 
{
	assert(dbp->write(file)==BLOCK_SIZE);
	cout << className << "::write(FILE*) test passed" << endl;
}

void* testReadBlock(FILE* file) 
{
	void* target = malloc(BLOCK_SIZE);
	assert(DataBlock::readBlock(file, target) == BLOCK_SIZE);
	cout << "DataBlock::readBlock(FILE*, void*) test passed" << endl;
	return target;
}

void testCreateBlock(void* target) 
{
	DataBlock* dabp = DataBlock::createBlock(target);
	testPutGet(dabp, "from createBlock: DataBlock");
	testIterator(dabp, "from createBlock: DataBlock");
	cout << "DataBlock from createBlock test passed" << endl;
}

void testIO(DataBlock* dbp, const char* fileName, const char* className) 
{
	FILE* file;
	file = fopen(fileName, "ab");
	testWrite(dbp, file, className);
	testWrite(dbp, file, className);
	testWrite(dbp, file, className);
	fclose(file);

	file = fopen(fileName, "rb");
	assert(fseek(file, BLOCK_SIZE, SEEK_SET) == 0);
	void* target = testReadBlock(file);
	testCreateBlock(target);

	assert(fseek(file, 2*BLOCK_SIZE, SEEK_SET) == 0);
	target = testReadBlock(file);
	testCreateBlock(target);

	assert(fseek(file, 0*BLOCK_SIZE, SEEK_SET) == 0);
	target = testReadBlock(file);
	testCreateBlock(target);
	fclose(file);
	cout << "I/O test for class " << className << " passed" << endl;
}
	
void testMetadata(DataBlock* dbp, BlockHeader* bhp, const char* className) 
{
	assert(dbp->getType() == bhp->type);
	assert((dbp->getRange()).lowerBound == bhp->range.lowerBound);
	assert((dbp->getRange()).upperBound == bhp->range.upperBound);
	assert(dbp->getNumEntries() == 2);
	assert(dbp->getDefaultValue() == 0);
	assert(dbp->put(0, R_ValueOfNA()) == NORMAL);
	assert(dbp->getNumEntries() == 1);
	assert(dbp->put(0, 10) == NORMAL);
	assert(dbp->getNumEntries() == 2);
	assert(dbp->put(0, -10) == NORMAL);
	assert(dbp->getNumEntries() == 2);
	cout << className << "block header metadata test passed" << endl;
}

void testBulkPut(DataBlock* dbp, const char* className)
{
   unsigned int n = 20;
   Entry entries[n];
   for(int k=0; k<n; k++) {
      setEntry(entries[k], k, 2*k);
   }
   Iterator* i = new EntryIterator(n, entries);
   assert(dbp->put(i) == NORMAL);
   delete i;

   Data datum = 0.0;
   for(int k=0; k<n; k++) {
      assert(dbp->get(k, datum) == NORMAL);
      assert(datum == 2*k);
   }
   cout << "Bulk put for " << className << "test passed" << endl;
}

int main()
{
	/*
	still need to test for: 
	*/
	setRange(r, 0, NUM_DENSE_ENTRIES-1);
	setBlockHeader(bhp, BLOCK_DENSE, r, 0, 0);
	for(int k=0; k<NUM_DENSE_ENTRIES; k++){
		*(dp+k)=R_ValueOfNA();
	}
	DenseBlock* dbp = new DenseBlock(bhp, dp);
  // DenseBlock* dbp = new DenseBlock(r, 1);
	testPutGet(dbp, "DenseBlock");
	testMetadata(dbp, bhp, "DenseBlock");
	testIterator(dbp, "DenseBlockIterator");
	testIO(dbp, "myBlock.bin", "DenseBlock");
	remove("myBlock.bin");
   testBulkPut(dbp, "DenseBlock");
   cout << "testing complete" << endl;
	delete dbp;
	return 0;
}
