#ifndef MDCOORD_H
#define MDCOORD_H

#include "../common/common.h"
#include <stdlib.h>
#include <stdarg.h>

#define COORD(x) ((i64)(x))

/**
 * MDCoord is a class for representing coordinates inside a
 * multi-dimensional array (MDArray).
 *
 * For extensibility, Coordinates use 64 bit signed integer
 * types. This makes it possible to do arithmetic operations on
 * MDCoord such as -. 32 bit integers are perhaps too small to
 * represent coordinates in a large 1-D array. Note that this does not
 * mean the index type for the linearized 1-D space should be 64 bit
 * or larger. As long as callers ensure no overflow or truncation
 * occurs, the 1-D linearized space can use a smaller integer type
 * like 32 bit.
 *
 * For guidelines of overloading operators such as =, ==, +=, refer to
 * http://www.cs.caltech.edu/courses/cs11/material/cpp/donnie/cpp-ops.html 
 */

class MDCoord
{
public:
    /// Number of dimensions.
    u8 nDim;
    /// Array of coordinates in each dimension.
    i64 *coords;

    /**
     * Constructs an empty MDCoord.
     */
    MDCoord()
    {
        nDim = 0;
        coords = NULL;
    }

    /**
     * Constructs a MDCoord using the given number of dimensions and
     * coordinates. It is the caller's responsibility to ensure the
     * number of coordinates given is equal to nDim.
     * 
     * \param nDim Number of dimensions
     * \param ... Variable length argument specifying the coordinates
     */
    
    MDCoord(u8 nDim, ...);

    /**
     * Constructs a MDCoord using the given coordinates and number of
     * dimensions. A copy of coord is kept.
     *
     * \param coords An array of coordinates in each dimension
     * \param nDim Number of dimensions
     */ 

    MDCoord(i64 *coords, u8 nDim);

    /**
     * Destructor. Memory resource for keeping the coordinates is
     * released.
     */
    
    ~MDCoord();

    /**
     * Copy constructor.
     *
     * \param src Source MDCoord object to be copied.
     */
    
    MDCoord(const MDCoord &src);

    /**
     * Assignment operator. Should be careful with self assignment.
     *
     * \param src Source MDCoord object to be copied.
     * \return A reference to self.
     */
    
    MDCoord & operator=(const MDCoord &src);

    /**
     * \name Comparison Operators
     */
    //@{

    /**
     * Equality comparison operator.
     *
     * \param other The object to be compared with.
     * \return true self is equal to other.
     */

    bool operator==(const MDCoord &other) const;

    /**
     * Non-equality comparison operator.
     *
     * \param other The object to be compared with.
     * \return true self is not equal to other.
     */
    
    bool operator!=(const MDCoord &other) const
    {
       return !(*this == other);
    }
    //@}
    
    /**
     * \name Compound Assignment Operators
     */
    //@{
    MDCoord & operator+=(const MDCoord &other);
    MDCoord & operator-=(const MDCoord &other);
    //@}

    /**
     * \name Binary Arithmetic Operators
     */
    //@{

    const MDCoord operator+(const MDCoord &other) const
    {
        return MDCoord(*this) += other;
    }
    
    const MDCoord operator-(const MDCoord &other) const
    {
        return MDCoord(*this) -= other;
    }

    //@}
    
};

#endif
