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

// If the file exists, initialize from the file;
// otherwise create a new one.
template<class Key_t, class Datum_t>
DirectlyMappedArray<Key_t, Datum_t>::DirectlyMappedArray(const char* fileName, 
        uint32_t numElements) { // where does numElements come from?
    BitmapPagedFile::createPagedFile(fileName, file); // should this be stored?
    this->numElements = numElements;
    buffer = new BufferManager<>(file, BUFFER_SIZE); 
}

template<class Key_t, class Datum_t>
DirectlyMappedArray<Key_t, Datum_t>::~DirectlyMappedArray() {
    delete file;
    delete buffer;
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
    ph.pid = (PID_t)key/PAGE_SIZE;
    buffer->readPage(ph);
    DenseArrayBlock<Datum_t> *dab = new DenseArrayBlock<Datum_t>(ph, 
            key - key%PAGE_SIZE, key - key%PAGE_SIZE + PAGE_SIZE);
    Datum_t result = dab->get(key);
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
    PID_t pid = (PID_t)key/PAGE_SIZE;
    if (buffer->allocatePageWithPID(pid, ph) != RC_SUCCESS) { /* page containing
       pid already exists */
       buffer->readPage(ph);
    }
    DenseArrayBlock<Datum_t> *dab = new DenseArrayBlock<Datum_t>(ph, 
        key - key%PAGE_SIZE, key - key%PAGE_SIZE + PAGE_SIZE);
    dab->put(pid, datum);
    buffer->flushPage(ph);
    delete dab;
}
