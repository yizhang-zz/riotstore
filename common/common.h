/** @file common.h */

#ifndef COMMON_H
#define COMMON_H

#define _in
#define _out

#include <unistd.h>
#include <stdlib.h>

/* Direct I/O, no caching by the OS */
#if defined(linux)
#define RIOT_LINUX
#include <malloc.h>
/* Linux supports O_DIRECT in open(2) for direct I/O */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <fcntl.h>
/// Open file in direct I/O mode.
#define open_direct(file, flag) open(file,(flag)|O_DIRECT, 0660)

#elif defined(__FreeBSD__)
#define RIOT_FREEBSD
#include <fcntl.h>
#define open_direct(file, flag) open(file,(flag)|O_DIRECT, 0660)

#elif defined(__APPLE__)
#define RIOT_APPLE
/* Mac OS support F_NOCACHE for direct I/O */
#include <fcntl.h>
int open_direct(const char *pathname, int flags);

#elif defined(sun)
#define RIOT_SUN
#include <stdlib.h>
/* Solaris has directio(3C) for the same purpose */
#include <fcntl.h>
int open_direct(const char *pathname, int flags);

#else
#error "Platform not supported"
#endif

#include <stdint.h>
#include <typeinfo>
#include <assert.h>
#include <map>

/* Utilities for the NA value. Taken from arithmetic.c from R src. */
typedef union
{
	double value;
	unsigned word[2];
} ieee_double;

/* gcc had problems with static const on AIX and Solaris
   Solaris was for gcc 3.1 and 3.2 under -O2 32-bit on 64-bit kernel */
#ifdef _AIX
#define CONST
#elif defined(sparc) && defined (__GNUC__) && __GNUC__ == 3
#define CONST
#else
#define CONST const
#endif

#ifdef WORDS_BIGENDIAN
static CONST int hw = 0;
static CONST int lw = 1;
#else  /* !WORDS_BIGENDIAN */
static CONST int hw = 1;
static CONST int lw = 0;
#endif /* WORDS_BIGENDIAN */

/// A special value representing NA of double type.
extern double	NA_DOUBLE;
/// A special value representing NA of int type.
extern int		NA_INT;

// Tests if a double value is NA.
bool isNA(double x);

/// Size of a disk block/page in bytes.
const int BLOCK_SIZE = 4096;
/// Size of a disk block/page in bytes.
const int PAGE_SIZE = BLOCK_SIZE;

//enum blockformat { dense, sparse };
//enum blocktype { leaf, internal};

/// \name Unsigned integers
//@{
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
//@}

/// \name Signed integers
//@{
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
//@}

/// Key type.
typedef uint32_t Key_t;
/// Key difference type.
typedef int32_t KeyDiff_t;
/// Datum type.
typedef double Datum_t;

/// A byte.
typedef uint8_t Byte_t;
/// A 32-bit integer used as page ID to uniquely identify a page in a file.
typedef uint32_t PID_t;
#define INVALID_PID (PID_t(-1))


struct Entry
{
	Key_t  	key;
	Datum_t 	datum;

	bool operator<(const Entry &other)
	{
		return key<other.key;
	}
};
/*
typedef struct 
{
	Key_t 	lowerBound;
	Key_t 	upperBound;
} Range;
*/

/*
typedef struct 
{
	BlockType		type;
	BlockFormat format;
	Range 	range;
	Datum_t 		default_value;
	uint32_t entry_count;
	BlockNo next;
} BlockHeader;

const int CAPACITY_DENSE  = ((BLOCK_SIZE - sizeof(BlockHeader))/sizeof(Datum_t));
const int CAPACITY_SPARSE = ((BLOCK_SIZE - sizeof(BlockHeader))/(sizeof(Datum_t)+sizeof(Key_t)));
*/


/// Return code type.
//typedef int RC_t;
//#define RC_SUCCESS 0
//#define RC_FAILURE 1
enum RC_t {
    RC_OK = 0,
    RC_Failure,
    RC_OutOfSpace,
    RC_NotAllocated,
    RC_AlreadyAllocated,
    RC_OutOfRange
};


//////////////////////////////////////////////////////////////////////
// In-memory image of a page.

typedef void *PageImage;
PageImage allocPageImage(size_t num);
void freePageImage(PageImage p);

//////////////////////////////////////////////////////////////////////
// A handle for a page buffered in memory.

typedef void *PageHandle;

/* Coding style: inline small functions (<= 10 lines of code) */

/* Calculates the capacity of a block */
/*
inline int BlockCapacity(BlockHeader *hdr)
{
	return ((hdr->format == DENSE) ? CAPACITY_DENSE : CAPACITY_SPARSE);
}
*/

/*
inline void SetRange(Range& range, Key_t lower, Key_t upper)
{
	range.lowerBound = lower;
	range.upperBound = upper;
}
*/

/*
inline void SetBlockHeader(BlockHeader* blockHeader, BlockFormat format, Range& range, BlockNo nextBlock, Datum_t def=0, uint32_t nEntries=0)
{
	blockHeader->format = format;
	blockHeader->range = range;
	blockHeader->next = nextBlock;
	blockHeader->default_value = def;
	blockHeader->entry_count = nEntries;
}
*/

/*
inline void SetEntry(Entry& entry, Key_t k, Datum_t d)
{
   entry.key = k;
   entry.datum = d;
}
*/
enum DataType { DT_INT, DT_DOUBLE, DT_COMPLEX };

/**
 * Tests if a value is of given type.
 * @param x a value.
 * @param t an enum value specifying the type.
 * @return true if x is of type t.
 */
template<class T>
inline bool IsSameDataType(T x, DataType t) {
	switch(t) {
case DT_INT:
	return typeid(x)==typeid(int);
case DT_DOUBLE:
	return typeid(x)==typeid(double);
case DT_COMPLEX:
	// to be implemented
	break;
default:
	return false;
	}
	return false;
}

/**
 * Gets the data type of a given value.
 * @param x a value of type T.
 * @return an enum specifying the type of x.
 */
template<class T>
inline DataType GetDataType(T x) {
	if (typeid(x) == typeid(int))
		return DT_INT;
	if (typeid(x) == typeid(double))
		return DT_DOUBLE;
	// to add complex type
	
	assert(false);
}

/**
 * Code returned from methods that access elements of an array.
 */
enum AccessCode {
    AC_OK = 0,
    AC_OutOfRange
};

// randomly permute an array of given size 
template<class T>
void permute(T* array, const int size)
{
    for (int i=0; i<size-1; i++) {
		// choose target from [i, size) randomly,
		// and swap i and target
        int target = rand() % (size-i) + i;
        T temp = array[target];
        array[target] = array[i];
        array[i] = temp;
    }
}

template<class T>
bool binarySearch(T *array, int size, T target, int *index)
{
	int p = 0;
	int q = size - 1;
	int mid;
	T midKey;

	do {
		mid = (p+q)/2;
		midKey = array[mid]; 
		if (midKey > target)
			q = mid-1;
		else
			p = mid+1;
	} while (p <= q && midKey != target);

	if (midKey == target) {
		*index = mid;
		return true;
	}
	else {
		*index = p;
		return false;
	}
}

// generate a k-permute from [begin, end] inclusive
template<class T>
void kPermute(T *array, T begin, T end, int k)
{
	std::map<T,T> htable;
	T size = end-begin+1;
	for (int i=0; i<k; i++) {
		int target = rand() % (size-i) + i;
		if (i==target)
			continue;
		if (htable.find(begin+i) == htable.end())
			htable[begin+i] = begin+i;
		if (htable.find(begin+target) == htable.end())
			htable[begin+target] = begin+target;
		T temp = htable[begin+i];
		htable[begin+i] = htable[begin+target];
		htable[begin+target] = temp;
	}
	for (int i=0; i<k; ++i) {
		if (htable.find(begin+i) == htable.end())
			array[i] = begin+i;
		else
			array[i] = htable[begin+i];
	}
}

void Error(const char *format, ...);

#ifdef DEBUG
void debug(const char *format, ...);
#else
#define debug(...)
#endif

inline int findFirstZeroBit(u32 word)
{
	int pos = 0;
	// bsf finds the least significant bit that is set (1)
	__asm__("bsfl %1,%0\n\t"
            "jne 1f\n\t"
            "movl $32, %0\n"
            "1:"
            : "=r" (pos)
            : "r" (~word));
	return pos;
}

#define TIMESTAMP(t1) \
	double t1; { timeval tim; gettimeofday(&tim, NULL); t1 = tim.tv_sec + tim.tv_usec/1000000.0;}
#endif
