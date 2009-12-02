#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <typeinfo>
#include <assert.h>

/* Utilities for the NA value; taken from arithmetic.c from R src. */
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

static double R_ValueOfNA()
{
   /* The gcc shipping with RedHat 9 gets this wrong without
    * the volatile declaration. Thanks to Marc Schwartz. */
   volatile ieee_double x;
   x.word[hw] = 0x7ff00000;
   x.word[lw] = 1954;
   return x.value;
};
bool R_IsNA(double x);

/* Array level definitions. */
enum DataType { INT, DOUBLE, COMPLEX };

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

template<class T>
inline DataType GetDataType(T x) {
	if (typeid(x) == typeid(int))
		return INT;
	if (typeid(x) == typeid(double))
		return DOUBLE;
	// to add complex type
	
	assert(false);
}

/* Block level definitions. */
const int BLOCK_SIZE = 4096;
const int PAGE_SIZE = BLOCK_SIZE;
#define PAGE_DENSE_CAP(x) (PAGE_SIZE/sizeof(x))
enum BlockFormat { DENSE, SPARSE };
enum BlockType { LEAF, INTERNAL};
typedef uint32_t Key_t;
typedef double Datum_t;
typedef uint32_t BlockNo; /* page number */

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

typedef struct 
{
	BlockType		type;
	BlockFormat format;
	Range 	range;
	Datum_t 		default_value;
	uint32_t entry_count;
	BlockNo next; /* only for leaves */
} BlockHeader;

const int CAPACITY_DENSE  = ((BLOCK_SIZE - sizeof(BlockHeader))/sizeof(Datum_t));
const int CAPACITY_SPARSE = ((BLOCK_SIZE - sizeof(BlockHeader))/(sizeof(Datum_t)+sizeof(Key_t)));

// const int PAGE_SIZE = 4096;
//////////////////////////////////////////////////////////////////////
// A byte.

typedef uint8_t Byte_t;

//////////////////////////////////////////////////////////////////////
// Return code.

typedef int RC_t;
#define RC_SUCCESS 0
#define RC_FAILURE 1

//////////////////////////////////////////////////////////////////////
// Page id, which uniquely identifies a page within a paged file.

typedef uint32_t PID_t;

#define PGID_INVALID UINT_MAX

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
inline int BlockCapacity(BlockHeader *hdr)
{
	return ((hdr->format == DENSE) ? CAPACITY_DENSE : CAPACITY_SPARSE);
}

inline void SetRange(Range& range, Key_t lower, Key_t upper)
{
	range.lowerBound = lower;
	range.upperBound = upper;
}

inline void SetBlockHeader(BlockHeader* blockHeader, BlockFormat format, Range& range, BlockNo nextBlock, Datum_t def=0, uint32_t nEntries=0)
{
	blockHeader->format = format;
	blockHeader->range = range;
	blockHeader->next = nextBlock;
	blockHeader->default_value = def;
	blockHeader->entry_count = nEntries;
}

inline void SetEntry(Entry& entry, Key_t k, Datum_t d)
{
   entry.key = k;
   entry.datum = d;
}

#endif
