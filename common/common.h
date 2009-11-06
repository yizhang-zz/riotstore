#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

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

/* Block level definitions. */
const int BLOCK_SIZE = 4096;
const int PAGE_SIZE = BLOCK_SIZE;
enum BlockFormat { DENSE, SPARSE };
enum BlockType { LEAF, INTERNAL, ROOT};
typedef uint32_t Key;
typedef double Datum;
typedef uint32_t BlockNo; /* page number */

typedef struct 
{
	Key  	key;
	Datum 	datum;
} Entry;

typedef struct 
{
	Key 	lowerBound;
	Key 	upperBound;
} Range;

typedef struct 
{
	BlockType		type;
	BlockFormat format;
	Range 	range;
	Datum 		default_value;
	uint32_t entry_count;
	BlockNo next; /* only for leaves */
} BlockHeader;

const int CAPACITY_DENSE  = ((BLOCK_SIZE - sizeof(BlockHeader))/sizeof(Datum));
const int CAPACITY_SPARSE = ((BLOCK_SIZE - sizeof(BlockHeader))/(sizeof(Datum)+sizeof(Key)));

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

inline void SetRange(Range& range, Key lower, Key upper)
{
	range.lowerBound = lower;
	range.upperBound = upper;
}

inline void SetBlockHeader(BlockHeader* blockHeader, BlockFormat format, Range& range, BlockNo nextBlock, Datum def=0, uint32_t nEntries=0)
{
	blockHeader->format = format;
	blockHeader->range = range;
	blockHeader->next = nextBlock;
	blockHeader->default_value = def;
	blockHeader->entry_count = nEntries;
}

inline void SetEntry(Entry& entry, Key k, Datum d)
{
   entry.key = k;
   entry.datum = d;
}

#endif
