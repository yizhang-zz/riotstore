#ifndef MDCOORD_H
#define MDCOORD_H

#include "../common/common.h"
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>

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


template<int nDim>
class MDCoord
{
	typedef i64 Coord;
private:
	Coord coords[nDim];

public:

    /**
     * Constructs an empty MDCoord.
     */
    MDCoord()
    {
		for (int i=0; i<nDim; ++i)
			coords[i] = 0;
    }

    /**
     * Constructs a MDCoord using the given number of dimensions and
     * coordinates. It is the caller's responsibility to ensure the
     * number of coordinates given is equal to nDim.
     * 
     * \param nDim Number of dimensions
     * \param ... Variable length argument specifying the coordinates
     */
    
	MDCoord(Coord x1)
	{
		for (int i=0; i<nDim; ++i)
			coords[i] = x1;
	}

    MDCoord(Coord x1, Coord x2)
	{
		coords[0] = x1;
		coords[1] = x2;
	}

    MDCoord(Coord x1, Coord x2, Coord x3)
	{
		coords[0] = x1;
		coords[1] = x2;
		coords[2] = x3;
	}

    /**
     * Constructs a MDCoord using the given coordinates and number of
     * dimensions. A copy of coord is kept.
     *
     * \param coords An array of coordinates in each dimension
     * \param nDim Number of dimensions
     */ 

    MDCoord(const Coord *coords)
	{
		memcpy(this->coords, coords, sizeof(Coord)*nDim);
	}


    /**
     * Copy constructor.
     *
     * \param src Source MDCoord object to be copied.
     */
    
    MDCoord(const MDCoord<nDim> &src)
	{
		//memcpy(coords, src.coords, sizeof(Coord)*nDim);
		for (int i=0; i<nDim; ++i)
			this->coords[i] = src.coords[i];
	}

    /**
     * Assignment operator. Should be careful with self assignment.
     *
     * \param src Source MDCoord object to be copied.
     * \return A reference to self.
     */
    
    MDCoord & operator=(const MDCoord<nDim> &src)
	{
		if (this != &src) {
			for (int i=0; i<nDim; ++i)
				coords[i] = src.coords[i];
			//memcpy(coords, src.coords, sizeof(Coord)*nDim);
		}
		return *this;
	}


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

    bool operator==(const MDCoord<nDim> &other) const
	{
		for (int i=0; i<nDim; ++i)
			if (other.coords[i] != coords[i])
				return false;
		return true;
	}

    /**
     * Non-equality comparison operator.
     *
     * \param other The object to be compared with.
     * \return true self is not equal to other.
     */
    
    bool operator!=(const MDCoord<nDim> &other) const
    {
       return !(*this == other);
    }
    //@}
    
    /**
     * \name Compound Assignment Operators
     */
    //@{
    MDCoord<nDim> & operator+=(const MDCoord<nDim> &other)
	{
		for (int i=0; i<nDim; ++i)
			coords[i] += other.coords[i];
		return *this;
	}

    MDCoord<nDim> & operator-=(const MDCoord<nDim> &other)
	{
		for (int i=0; i<nDim; ++i)
			coords[i] -= other.coords[i];
		return *this;
	}

    //@}

    /**
     * \name Binary Arithmetic Operators
     */
    //@{

    MDCoord<nDim> operator+(const MDCoord<nDim> &other) const
    {
        return MDCoord<nDim>(*this) += other;
    }
    
    MDCoord<nDim> operator-(const MDCoord<nDim> &other) const
    {
        return MDCoord<nDim>(*this) -= other;
    }

    //@}

	i64& operator[] (unsigned i)
	{
		return coords[i];
	}

	const i64& operator[] (unsigned i) const
	{
		return coords[i];
	}

	MDCoord<nDim> max(const MDCoord<nDim> &other) const
	{
		MDCoord<nDim> ret(*this);
		for (int i=0; i<nDim; ++i)
			if (ret[i] < other[i])
				ret[i] = other[i];
		return ret;
	}

	MDCoord<nDim> min(const MDCoord<nDim> &other) const
	{
		MDCoord<nDim> ret(*this);
		for (int i=0; i<nDim; ++i)
			if (ret[i] > other[i])
				ret[i] = other[i];
		return ret;
	}

    MDCoord<nDim> transpose() const
    {
        MDCoord<nDim> ret;
        for (int i=0; i<nDim/2; ++i) {
            ret[i] = coords[nDim-1-i];
            ret[nDim-1-i] = coords[i];
        }
        return ret;
    }

	/*
	i64* operator&() const
	{
		return coords;
	}
	 */
    std::string toString() const
	{
		char buf[256] = "[";
		char num[20];
		if (nDim > 0) {
			sprintf(num, "%" PRId64, coords[0]);
			strcat(buf, num);
		}
		for (int i=1; i<nDim; i++) {
			sprintf(num, " %" PRId64, coords[i]); 
			strcat(buf, num);
		}
		strcat(buf, "]");
		return std::string(buf);
	}

	static MDCoord<nDim> parse(char **p)
	{
		int *q = (int*) *p;
		*p += sizeof(int);
		assert(nDim==*q);
		Coord *r = (Coord*) *p;
		MDCoord<nDim> coord;
		for (int i=0; i<*q; ++i)
			coord[i] = r[i];
		*p += sizeof(Coord) * (*q);
		return coord;
	}

	void serialize(char **p)
	{
		int *q = (int*) *p;
		*q = nDim;
		*p += sizeof(int);
		Coord *r = (Coord*) *p;
		for (int i=0; i<nDim; ++i)
			r[i] = coords[i];
		*p += sizeof(Coord) * nDim;
	}

	template<int N>
	friend std::ostream & operator<<(std::ostream &out, const MDCoord<N> &coord);
};

template<int N>
std::ostream & operator<<(std::ostream &out, const MDCoord<N> &coord)
{
	return out<<coord.toString();
}

template<int nDim>
struct MDArrayElement
{
    MDCoord<nDim> coord;
    Datum_t datum;

    /**
     * Compares in column major order.
     */
    bool operator< (const MDArrayElement<nDim> &other) const
    {
        for (int i=nDim-1; i>=0; --i) {
            if (coord[i] < other.coord[i])
                return true;
            else if (coord[i] > other.coord[i])
                return false;
        }
        return false;
    }
};

#endif
