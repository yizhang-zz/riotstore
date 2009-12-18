#ifndef LINEAR_STORAGE_H
#define LINEAR_STORAGE_H

#include "common.h"
#include "ArrayInternalIterator.h"
#include "../lower/PagedStorageContainer.h"
#include "../lower/BufferManager.h"

enum StorageType {
    DMA,
    BTREE
};

/**
 * A storage device for linear (1-D) arrays. Used as the underlying
 * storage for multi-dimensional arrays.  This class cannot be
 * instantiated.  Programmers have to write concrete subclasses.
 *
 * Examples of LinearStorage include directly mapped arrays and
 * B-trees.
 *
 * \sa MDArray
 */ 
class LinearStorage
{
protected:
    /// Total size, including empty slots
    u32 size;
    /// Physical storage
    PagedStorageContainer *file;
    /// Buffer manager
    BufferManager<> *buffer;

public:

    /**
     * Virtual destructor.
     */
    virtual ~LinearStorage() {}

    /**
     * Gets an entry with the specified key.
     *
     * \param [in] key Key of the entry.
     * \param [out] datum Datum to be returned.
     * \return OK if successful, OutOfRange if index out of range.
     */
    virtual int get(Key_t &key, Datum_t &datum) = 0;

    /**
     * Sets the datum for the specified key.
     *
     * \param [in] key Key of the entry.
     * \param [in] datum New datum to be put.
     * \return OK if successful, OutOfRange if index out of range.
     */
    virtual int put(Key_t &key, Datum_t &datum) = 0;

    /**
     * Creates an internal iterator.
     *
     * \param t Type of iterator.
     * \return An internal iterator.
     */
    virtual ArrayInternalIterator* createIterator(IteratorType t) = 0;    
};

#endif
