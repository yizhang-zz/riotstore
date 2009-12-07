#ifndef BTREE_BLOCK_H
#define BTREE_BLOCK_H

#include "../common/common.h"
#include <string.h>

#define BT_OK 0
#define BT_OVERWRITE 1
#define BT_OVERFLOW 2

#define BT_FOUND 0
#define BT_NOT_FOUND 1

/*
 * Structure of a Btree block: 
 * OFFSET	SIZE	DESC
 * 0		1		Flags. bit 1: is leaf, bit 2: is dense. 
 * 1		2		Number of entries. For internal nodes, this excludes
 * 					the extra child pointer.
 * --- the following are optional ---
 * 3		4		Page ID of next leaf; only for leaf blocks.
 * 3		4		Rightmost child page ID; only for internal blocks.
 * 
 * The lower and upper bounds of a block is not explicitly stored to save
 * space. They can be derived from the keys in the parent block.
 *
 * A dense leaf block contains data that are densely packed. From low
 * address space are entry 0, 1, 2, ...
 * 
 * A sparse leaf block contains both keys and data. Keys begin right after
 * the header and grow towards the high address end, while data items start
 * from the high-end of the address space of the block and grow downwards.
 * The rational behind this design is better cache performance when
 * searching, because the keys are collocated. For now, there is no prefix
 * compression on keys because long keys often dominate anyway.
 *
 * An internal block contains both keys and PIDs of child blocks. Keys begin
 * right after the header and grow towards the high address end, while PIDs
 * start from the high end and grow downwards. The relationship between keys
 * and PIDs: lower_bound <= p0 < k0 <= p1 < k1 <= p2 < ... <= pn <
 * upper_bound. There is one more PID than keys; therefore, the rightmost
 * PID is stored in the header as specified above.
 */

class BtreeBlock {
public:
	u16		nEntries;	/* how many entries */
	Key_t	lower;		/* all keys >= lower */
	Key_t	upper;		/* all keys < upper */
	bool	isLeaf;
	bool	isDense;
	PID_t	*nextLeaf;
	PID_t	*rightChild;
	PageHandle	ph;

	struct OverflowEntry {
		// insert this entry before the idx-th non-overflow entry
		u16 idx;
		// body of the overflown entry
		u8 data[8];
	} overflowEntries[2];

	const static u16 headerSize=8;	/* should be 7 but 8 makes alignment
									   easier */
	/* If a key is not found in a dense leaf block, it has a default value
	 * and is omitted. */
	const static Datum_t defaultValue = 0.0;

	static u16		denseCap;
	static u16		sparseCap;
	static u16		internalCap;

	BtreeBlock(PageHandle *pPh, Key_t beginsAt, Key_t endsBy);
	BtreeBlock(PageHandle *pPh, Key_t beginsAt, Key_t endsBy, bool isLeaf,
			bool isDense, bool isRoot=false);
	// ~BtreeBlock();

	int search(Key_t key, u16 *idx);

	int put(Key_t key, void *p);

	int get(Key_t key, void *p);
};

#endif
