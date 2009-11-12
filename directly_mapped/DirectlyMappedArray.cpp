#include "DirectlyMappedArray.h"



// Initializes the block by reading from a page image.  The index
// range and the default data value will be passed in---the caller,
// with the knowledge of the overall DirectlyMappedArray, should
// know the exact index range that this page is responsible for.
// The default data value will also be supplied by the caller.
template<class Datum_t>
DenseArrayBlock<Datum_t>::DenseArrayBlock(PageHandle *ph, Key_t lower, Key_t upper) {
    this->ph = ph;
    this->lowerBound = lower;
    this->upperBound = upper;
    data = (Datum_t *) ph->image;
}

template<class Datum_t>
DenseArrayBlock<Datum_t>::~DenseArrayBlock() {
}

// Datum_t getDefaultValue() const;

template<class Datum_t>
Datum_t DenseArrayBlock<Datum_t>::get(Key_t key) const { // assume key is within range
    return *(data + key - this->lowerBound);
}

// assume key is within range
template<class Datum_t>
void DenseArrayBlock<Datum_t>::put(Key_t key, Datum_t datum) { 
    *(data + key - this->lowerBound) = datum;
}


template<class Key_t, class Datum_t>
DirectlyMappedArray<Key_t, Datum_t>::DirectlyMappedArray(const char* fileName, 
        uint32_t numElements) {
	if (numElements > 0) {		// new array to be created
		remove(fileName);
		BitmapPagedFile::createPagedFile(fileName, file);
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
	else {						// existing array
		if (access(fileName, F_OK) != 0)
			throw std::string("File for array does not exist.");
		BitmapPagedFile::createPagedFile(fileName, file);
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

template<class Key_t, class Datum_t>
DirectlyMappedArray<Key_t, Datum_t>::~DirectlyMappedArray() {
	// should delete buffer first, because flushAll() is called in
	// buffer's destructor, at which time file is updated.
    delete buffer;
    delete file;
}

/**
  * check if key is in range
  * if it is, read page containing key
  * wrap page as DenseArrayBlock, calling get
  */
template<class Key_t, class Datum_t>
Datum_t DirectlyMappedArray<Key_t, Datum_t>::get(Key_t key) const {
    if (key < 0 || numElements <= key) {
        return R_ValueOfNA(); // key out of range
    }

    PageHandle ph;
	findPage(key, &ph.pid);
    buffer->readPage(ph);
    DenseArrayBlock<Datum_t> *dab = new DenseArrayBlock<Datum_t>(ph, 
            key - key%PAGE_SIZE, key - key%PAGE_SIZE + PAGE_SIZE);
    Datum_t result = dab->get(key);
	buffer->unpinPage(ph);
    delete dab;
    return result;
}

/**
  * check if already page that should contain key on file
  * if so, grab that page, otherwise create new page
  * wrap page as DenseArrayBlock, then call put on DAB
  * flush page?
  */
template<class Key_t, class Datum_t>
void DirectlyMappedArray<Key_t, Datum_t>::put(Key_t key, Datum_t datum) {
    PageHandle ph;
	findPage(key, &ph.pid);
    if (buffer->allocatePageWithPID(ph.pid, ph) != RC_SUCCESS) { /* page containing
       pid already exists */
       buffer->readPage(ph);
    }
    DenseArrayBlock<Datum_t> *dab = new DenseArrayBlock<Datum_t>(ph, 
        key - key%PAGE_SIZE, key - key%PAGE_SIZE + PAGE_SIZE);
    dab->put(key, datum);
    buffer->flushPage(ph);
	buffer->unpinPage(ph);
    delete dab;
}

