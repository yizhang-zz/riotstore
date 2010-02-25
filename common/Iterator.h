#ifndef ITERATOR_H
#define ITERATOR_H

#include "common.h"


/// Type of iterator.
enum IteratorType {
    Dense,
    Sparse
};

/**
 * An iterator over a collection of array elements.  It allows the
 * programmer to traverse the collection in either direction along a
 * predefined order.  During the traversal, elements pointed to by the
 * iterator can be read or modified.  It serves as a common interface
 * for various concrete iterator implementations such as those for
 * multi-dimensional arrays or B-tree blocks.
 *
 * The collection an iterator acts on may be multi-dimensional
 * (non-linear), in which case there is no default order for which
 * next() and prev() are defined.  In such cases, class Linearization
 * must be used to help make iterators properly defined.
 *
 * When an iterator is initialized, it points to the position right
 * before the first entry in the collection.  Thus moveNext() must be
 * called to get the first entry.  When the iterator points to an
 * invalid position, i.e., before the first entry or after the last
 * entry, the result of get() is undefined.
 */

template <class K, class D>
class Iterator
{
    // typedef Iterator<K, D> Self;
public:
    
    /**
     * Moves the iterator to the next entry in the collection.
     *
     * \return true if successfully moved to the next entry; false if
     * passed the end of the collection.
     */
    virtual bool moveNext() = 0;

    /**
     * Moves the iterator to the previous entry in the collection.
     *
     * \return true if successfully moved to the previous entry; false
     * if already before the beginning of the collection.
     */
    virtual bool movePrev() { return false ; }

    /**
     * Gets the key and datum the iterator currently points to.
     *
     * \param [out] k Key.
     * \param [out] d Datum.
     */
    virtual void get(K &k, D &d) = 0;
    
    /**
     * Replaces the last datum returned by next() or prev() with the
     * specified datum.  If supplied with a datum of value 0.0, a
     * subclass implementation may interpret the action as a deletion
     * if it decides not to explicitly store zero values.
     *
     * An add method is not necessary for an iterator because an array
     * has one slot for each of its elements, at least conceptually.
     *
     * \param [in] d Datum to be put.
     */ 
    virtual void put(const D &d) = 0;

    /**
     * Resets the iterator to its initial position, which is before
     * the first entry in the collection.
     */
    virtual void reset() = 0;

    /**
     * Virtual destructor.
     */
    virtual ~Iterator() {}

    /**
     * Sets the range of the iteration to [beginsAt, endsBy).
     *
     * \param beginsAt Key of first entry.
     * \param endsBy Key of last entry plus 1.
     * \return true if successful; false if out of range or beginsAt
     * is not before endsBy.
     */
    virtual bool setRange(const K &beginsAt, const K &endsBy)
    {
        this->beginsAt = beginsAt;
        this->endsBy = endsBy;
        return false;
    }

    virtual bool setIndexRange(int begin, int end)
    {
        return false;
    }

protected:
    /// Where the iteration should begin (inclusive).
    K beginsAt;
    /// Where the iteration should end (exclusive).
    K endsBy;
};

#endif
