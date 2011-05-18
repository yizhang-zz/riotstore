/** @file common.h */

#ifndef COMMON_H
#define COMMON_H

#define _in
#define _out

#define STORAGE_METADATA_PAGES 271
/*
#if defined(_LP64) || defined(__LP64__)
#define D64 "ld"
#else
#define D64 "lld"
#endif
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <assert.h>
#include <map>
#include <iostream>
#include <typeinfo>
#include <iterator>
#include <boost/shared_ptr.hpp>
#include <gsl/gsl_rng.h>

int riot_open(const char *pathname, int flags);

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
const size_t BLOCK_SIZE = 4096*2;
/// Size of a disk block/page in bytes.
const size_t PAGE_SIZE = BLOCK_SIZE;

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
typedef uint64_t Key_t;
#define MAX_KEY (~((Key_t) 0))
/// Key difference type.
typedef int64_t KeyDiff_t;
/// Datum type.
typedef double Datum_t;

/// A byte.
typedef uint8_t Byte_t;
/// A 32-bit integer used as page ID to uniquely identify a page in a file.
typedef uint32_t PID_t;
#define INVALID_PID (PID_t(-1))

struct Entry
{
	Key_t key;
	union {
		Datum_t datum;
		Datum_t *pdatum;
	};

	Entry()
	{}
    Entry(Key_t k) : key(k)
    {}
	Entry(Key_t k, Datum_t d) : key(k), datum(d)
	{}
	bool operator<(const Entry &other) const
	{
		return key<other.key;
	}

	friend std::ostream & operator<<(const std::ostream &out, const Entry &e);
};

inline std::ostream & operator<<(std::ostream &out, const Entry &e)
{
	return out<<e.key;
}

// Use Entry instead of KVPair_t. KVPair_t requires setting the datum pointer; might as well use Entry and copy the actual datum to Entry.
/*
typedef struct
{
    Key_t key;
    Datum_t *datum;
} KVPair_t;

int compareKVPair(const void *a, const void *b);
*/

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
typedef int RC_t;
#define RC_OK               0x8000
#define RC_READ             0x8001
#define RC_ALLOC            0x8002
#define RC_HIT              0x8003
#define RC_FAIL             0x4000
#define RC_OutOfSpace       0x4001   
#define RC_NotAllocated     0x4002
#define RC_AlreadyAllocated 0x4003
#define RC_OutOfRange       0x4004


//////////////////////////////////////////////////////////////////////
// In-memory image of a page.

typedef void *PageImage;
PageImage allocPageImage(size_t num);
void freePageImage(PageImage p);

//////////////////////////////////////////////////////////////////////
// A handle for a page buffered in memory.

//typedef void *PageHandle;
struct PageRec;
typedef PageRec *PageHandle;
//typedef boost::shared_ptr<PageRec> PageHandle;

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
    gsl_rng *r = gsl_rng_alloc(gsl_rng_taus2);
    for (int i=0; i<size-1; i++) {
		// choose target from [i, size) randomly,
		// and swap i and target
        int target = gsl_rng_uniform_int(r, size-i) + i;
        T temp = array[target];
        array[target] = array[i];
        array[i] = temp;
    }
    gsl_rng_free(r);
}

template<class Iterator>
bool binarySearch(Iterator begin, Iterator end, 
		typename std::iterator_traits<Iterator>::value_type target, int *index)
{
	Iterator p = begin;
	end--;
	Iterator mid;

	while (p <= end) {
		mid = p+(end-p)/2;
		if (*mid == target) {
			*index = mid - begin;
			return true;
		}
		else if (*mid > target)
			end = mid - 1;
		else
			p = mid + 1;
	}

	*index = p - begin;
	return false;
}

// generate a k-permute from [begin, end] inclusive
template<class T>
void kPermute(T *array, T begin, T end, int k)
{
    gsl_rng *r = gsl_rng_alloc(gsl_rng_taus2);
	std::map<T,T> htable;
	T size = end-begin+1;
	for (int i=0; i<k; i++) {
		int target = gsl_rng_uniform_int(r, size-i) + i;
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

#define STRINGIZE(x) STRINGIZE2(x) 
#define STRINGIZE2(x) #x 
#define LINE_STRING STRINGIZE(__LINE__) 

#define Error( ...) do { \
   	fprintf(stderr, "[ERROR] " __FILE__ ":" LINE_STRING " " __VA_ARGS__); \
   	fprintf(stderr, "\n"); \
} while (0)

#ifdef DEBUG
//#define Debug(format, ...) fprintf(stderr, "[DEBUG] " __FILE__ ":%d " format "\n", __LINE__, ##__VA_ARGS__)
#define Debug( ...) do { \
   	fprintf(stderr, "[DEBUG] " __FILE__ ":" LINE_STRING " " __VA_ARGS__); \
   	fprintf(stderr, "\n"); \
} while (0)
#else
#define Debug(...)
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

inline int floorLog2(unsigned int n) {
	if (n == 0)
		return -1;

	int pos = 0;
	if (n >= 1<<16) { n >>= 16; pos += 16; }
	if (n >= 1<< 8) { n >>=  8; pos +=  8; }
	if (n >= 1<< 4) { n >>=  4; pos +=  4; }
	if (n >= 1<< 2) { n >>=  2; pos +=  2; }
	if (n >= 1<< 1) {           pos +=  1; }
	return pos;
}

inline int ceilingLog2(unsigned int n) {
    int pos = floorLog2(n);
    return n==1U<<pos ? pos : pos+1;
}

#define TIMESTAMP(t1) \
	double t1; do { timeval tim; gettimeofday(&tim, NULL); t1 = tim.tv_sec + tim.tv_usec/1000000.0;} while(0)
#endif
