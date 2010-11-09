#ifndef LINEAR_STORAGE_H
#define LINEAR_STORAGE_H

#include "../common/common.h"
#include "../common/ArrayInternalIterator.h"
#include "PagedStorageContainer.h"
#include "BufferManager.h"

class MDArray;

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
    /// Upper bound of stored keys
    Key_t upper;
    /// Physical storage
    PagedStorageContainer *file;
    /// Buffer manager
    BufferManager *buffer;

public:

#ifdef PROFILING
    static int readCount;
    static int writeCount;
    static double accessTime;

    static void resetPerfCounts()
    {
        readCount = writeCount = 0;
        accessTime = 0.0;
    }
#endif
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
    virtual int get(const Key_t &key, Datum_t &datum) = 0;

    /**
     * Sets the datum for the specified key.
     *
     * \param [in] key Key of the entry.
     * \param [in] datum New datum to be put.
     * \return OK if successful, OutOfRange if index out of range.
     */
    virtual int put(const Key_t &key, const Datum_t &datum) = 0;

    virtual int batchPut(i64 putCount, const KVPair_t *puts) = 0;

    /**
     * Creates an internal iterator.
     *
     * \param t Type of iterator.
     * \return An internal iterator.
     */
    virtual ArrayInternalIterator* createIterator(IteratorType t, Key_t &beginsAt, Key_t &endsBy) = 0;    

	Key_t getUpperBound()
	{
		return upper;
	}

    friend class MDArray;
};

#endif
