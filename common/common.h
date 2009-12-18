/** @file common.h */

#ifndef COMMON_H
#define COMMON_H

/// Print error message like printf.
void riot_error(const char *s, ...);

/* Direct I/O, no caching by the OS */
#if defined(linux)
#define RIOT_LINUX
/* Linux supports O_DIRECT in open(2) for direct I/O */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <fcntl.h>
/// Open file in direct I/O mode.
#define open_direct(file, flag) open(file,(flag)|O_DIRECT, 0660)

#elif defined(BSD)||defined(__APPLE__)
#define RIOT_BSD
/* BSD & Mac OS support F_NOCACHE for direct I/O */
#include <fcntl.h>
int open_direct_bsd(const char *pathname, int flags);
/// Open file in direct I/O mode.
#define open_direct(file, flag) open_direct_bsd(file, flag)

#elif defined(sun)
#define RIOT_SUN
/* Solaris has directio(3C) for the same purpose */
#include <fcntl.h>
int open_direct_sol(const char *pathname, int flags);
/// Open file in direct I/O mode.
#define open_direct(file, flag) open_direct_sol(file, flag)
#endif

#include <stdint.h>
#include <typeinfo>
#include <assert.h>

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

static bool R_IsNA(double x);
/// Tests if a double value is NA.
#define ISNA(x) R_IsNA(x)

/// Size of a disk block/page in bytes.
const int BLOCK_SIZE = 4096;
/// Size of a disk block/page in bytes.
const int PAGE_SIZE = BLOCK_SIZE;

enum BlockFormat { DENSE, SPARSE };
enum BlockType { LEAF, INTERNAL};

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

typedef struct 
{
	Key_t  	key;
	Datum_t 	datum;
} Entry;

typedef struct 
{
	Key_t 	lowerBound;
	Key_t 	upperBound;
} Range;

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
typedef int RC_t;
#define RC_SUCCESS 0
#define RC_FAILURE 1

// #define PGID_INVALID UINT_MAX

//////////////////////////////////////////////////////////////////////
// In-memory image of a page.

typedef Byte_t PageImage[PAGE_SIZE];

//////////////////////////////////////////////////////////////////////
// A handle for a page buffered in memory.

typedef struct {
  PID_t pid;
  PageImage *image;
} PageHandle;


/* Coding style: inline small functions (<= 10 lines of code) */

/* Calculates the capacity of a block */
/*
inline int BlockCapacity(BlockHeader *hdr)
{
	return ((hdr->format == DENSE) ? CAPACITY_DENSE : CAPACITY_SPARSE);
}
*/

inline void SetRange(Range& range, Key_t lower, Key_t upper)
{
	range.lowerBound = lower;
	range.upperBound = upper;
}

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

inline void SetEntry(Entry& entry, Key_t k, Datum_t d)
{
   entry.key = k;
   entry.datum = d;
}

enum DataType { INT, DOUBLE, COMPLEX };

/**
 * Tests if a value is of given type.
 * @param x a value.
 * @param t an enum value specifying the type.
 * @return true if x is of type t.
 */
template<class T>
inline bool IsSameDataType(T x, DataType t) {
	switch(t) {
case INT:
	return typeid(x)==typeid(int);
case DOUBLE:
	return typeid(x)==typeid(double);
case COMPLEX:
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
		return INT;
	if (typeid(x) == typeid(double))
		return DOUBLE;
	// to add complex type
	
	assert(false);
}

/**
 * Code returned from methods that access elements of an array.
 */
enum AccessCode {
    OK = 0,
    OutOfRange
};

#endif
