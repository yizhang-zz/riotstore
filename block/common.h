#ifndef COMMON_H
#define COMMON_H

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
enum BlockFormat { DENSE, SPARSE };
enum BlockType { LEAF, INTERNAL, ROOT};
typedef uint32_t key_t;
typedef double datum_t;
typedef uint32_t pgno_t; /* page number */

typedef struct 
{
	key_t  	key;
	datum_t 	datum;
} Entry;

typedef struct 
{
	key_t 	lowerBound;
	key_t 	upperBound;
} Range;

typedef struct 
{
	BlockType		type;
	BlockFormat format;
	Range 	range;
	Datum 		default_value;
	uint32_t entry_count;
	pgno_t next; /* only for leaves */
} BlockHeader;

const int CAPACITY_DENSE  = ((BLOCK_SIZE - sizeof(BlockHeader))/sizeof(datum_t));
const int CAPACITY_SPARSE = ((BLOCK_SIZE - sizeof(BlockHeader))/sizeof(datum_t+key_t));

/* Coding style: inline small functions (<= 10 lines of code) */

/* Calculates the capacity of a block */
inline int block_capacity(BlockHeader *hdr)
{
	return ((hdr->format == DENSE) ? CAPACITY_DENSE : CAPACITY_SPARSE);
}

inline void set_range(Range& range, key_t lower, key_t upper)
{
	range.lowerBound = lower;
	range.upperBound = upper;
}

inline void set_block_header(BlockHeader* blockHeader, int type, Range& range, pgno_t nextBlock, datum_t def=0, uint32_t nEntries=0)
{
	blockHeader->type = type;
	blockHeader->range = range;
	blockHeader->next = nextBlock;
	blockHeader->default_value = def;
	blockHeader->entry_count = nEntries;
}

inline void set_entry(Entry& entry, key_t k, datum_t d)
{
   entry.key = k;
   entry.data = d;
}

#endif