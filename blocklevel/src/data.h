#ifndef DATA_H
#define DATA_H

/* taken from arithmetic.c from R src */
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


#define BLOCK_SIZE 4096
#define NUM_DENSE_ENTRIES ((BLOCK_SIZE - sizeof(BlockHeader))/sizeof(Data))
enum BlockType { BLOCK_NULL, BLOCK_DENSE, BLOCK_SPARSE };

typedef unsigned Key;
typedef double Data;

typedef struct 
{
	Key  	key;
	Data 	data;
} Entry;

typedef struct 
{
	Key 	lowerBound;
	Key 	upperBound;
} Range;

typedef struct 
{
	int		type;
	Range 	range;
	unsigned	nextBlock;
	Data 		defaultValue;
	unsigned nEntries;
} BlockHeader;

typedef struct 
{
	/* key is 4-byte, so reserve 4 bytes for the prefix; will be enough */
	unsigned prefix;
	unsigned compressedKeyLength; /* how many bytes the compressed key takes */
} SparseHeader;


void setRange(Range& range, Key lower, Key upper);
void setBlockHeader(BlockHeader* blockHeader, int type, Range range, unsigned nextBlock, Data def=0, unsigned nEntries=0); 
void setEntry(Entry& entry, Key k, Data d);

#endif
