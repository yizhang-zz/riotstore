
#include <iostream>
#include "../lower/LRUPageReplacer.h"
#include "../lower/BitmapPagedFile.h"
#include "DMADenseIterator.h"
#include "DMASparseIterator.h"
#include "DirectlyMappedArray.h"


/// If numElements > 0, create a new array; otherwise read from disk.
/// Whether file exists is ignored.
DirectlyMappedArray::DirectlyMappedArray(const char* fileName, uint32_t numElements) 
{
   if (numElements > 0)		// new array to be created
   {
      remove(fileName);
      file = new BitmapPagedFile(fileName, BitmapPagedFile::F_CREATE);
      buffer = new BufferManager<>(file, BUFFER_SIZE); 
      this->numElements = numElements;
      PageHandle ph;
      assert(RC_SUCCESS == buffer->allocatePageWithPID(0, ph));
      // page is already marked dirty
      DirectlyMappedArrayHeader* header = (DirectlyMappedArrayHeader*) ph.image;
      header->numElements = numElements;
      Datum_t x;
      header->dataType = GetDataType(x);
      buffer->unpinPage(ph);
   }
   else 						// existing array
   {
      if (access(fileName, F_OK) != 0)
         throw ("File for array does not exist.");
      file = new BitmapPagedFile(fileName, BitmapPagedFile::F_NO_CREATE);
      buffer = new BufferManager<>(file, BUFFER_SIZE); 
      PageHandle ph;
      ph.pid = 0; 			// first page is header
      buffer->readPage(ph);
      DirectlyMappedArrayHeader header = *((DirectlyMappedArrayHeader*) ph.image);
      buffer->unpinPage(ph);
      this->numElements = header.numElements;
      Datum_t x;
      assert(IsSameDataType(x, header.dataType));
   }
}

   /// should delete buffer first, because flushAll() is called in
   /// buffer's destructor, at which time file is updated.
DirectlyMappedArray::~DirectlyMappedArray() 
{
   delete buffer;
   delete file;
}

int DirectlyMappedArray::get(Key_t &key, Datum_t &datum) 
{
   if (key < 0 || numElements <= key) 
   {
      return NA_DOUBLE; // key out of range
   }

   PageHandle ph;
   findPage(key, &(ph.pid));
   buffer->readPage(ph);
   DenseArrayBlock *dab = new DenseArrayBlock(&ph, 
         (key/CAPACITY)*CAPACITY, (key/CAPACITY+1)*CAPACITY);
   datum = dab->get(key);
   buffer->unpinPage(ph);
   delete dab;
   return RC_SUCCESS;
}

int DirectlyMappedArray::put(Key_t &key, Datum_t &datum) 
{
   PageHandle ph;
   findPage(key, &(ph.pid));
   if (buffer->allocatePageWithPID(ph.pid, ph) != RC_SUCCESS) 
   { /* page containing pid already exists */
      buffer->readPage(ph);
   }
   DenseArrayBlock *dab = new DenseArrayBlock(&ph, 
         (key/CAPACITY)*CAPACITY, (key/CAPACITY+1)*CAPACITY);
   dab->put(key, datum);
   buffer->markPageDirty(ph);
   buffer->unpinPage(ph);
   delete dab;
   return RC_SUCCESS;
}

ArrayInternalIterator *DirectlyMappedArray::createIterator(IteratorType t, Key_t beginsAt, Key_t endsBy)
{
   if (t == Dense)
      return new DMADenseIterator(beginsAt, endsBy, this);
   else if (t == Sparse)
      return NULL;
   // return new DMASparseIterator(beginsAt, endsBy, this);
}

void DirectlyMappedArray::findPage(Key_t &key, PID_t *pid) 
{
   *pid = key/CAPACITY + 1;
}

RC_t DirectlyMappedArray::loadBlock(PID_t pid, DenseArrayBlock** block) 
{
   PageHandle ph;
   ph.pid = pid;
   if (buffer->readPage(ph) == RC_FAILURE)
      return RC_FAILURE;
   *block = new DenseArrayBlock(&ph, CAPACITY*(pid-1), CAPACITY*pid);
   return RC_SUCCESS;
}

RC_t DirectlyMappedArray::releaseBlock(DenseArrayBlock* block) 
{
   return buffer->unpinPage((block->ph));
}

uint32_t DirectlyMappedArray::getLowerBound() 
{
   return 0;
}

uint32_t DirectlyMappedArray::getUpperBound() 
{
   return numElements;
}
