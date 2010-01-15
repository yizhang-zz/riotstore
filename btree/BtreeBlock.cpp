
#include "Btree.h"

#include "../common/common.h"
#include "BtreeBlock.h"
#include "BtreeDLeafBlock.h"
#include "BtreeSLeafBlock.h"
#include "BtreeIntBlock.h"
#include <iostream>

/* 
 * Initialize static members.
 */
/*
u16 BtreeBlock::denseCap = (PAGE_SIZE -
		BtreeBlock::headerSize)/(sizeof(Datum_t));
u16 BtreeBlock::sparseCap = (PAGE_SIZE -
		BtreeBlock::headerSize)/(sizeof(Key_t)+sizeof(Datum_t));
u16 BtreeBlock::internalCap = (PAGE_SIZE -
		BtreeBlock::headerSize)/(sizeof(Key_t)+sizeof(PID_t));
*/


/*
 * Initializes the block by reading from a page image. The key range is
 * provided since the caller, with the knowledge of the overall tree, should
 * know the exact key range that this block is responsible for.
 */
BtreeBlock::BtreeBlock(PageHandle *pPh, Key_t beginsAt, Key_t endsBy,
                       bool create)
{
	ph = *pPh;		// make a physical copy
	lower = beginsAt;
	upper = endsBy;

	u8 *pData = *ph.image;
    nEntries = (u16*) (pData+1);
    
    if (create) {
        *nEntries = 0;
    }
}

BtreeBlock *BtreeBlock::load(PageHandle *pPh, Key_t beginsAt, Key_t endsBy)
{
    BtreeBlock *block;
    u8 flags = (*pPh->image)[0];
    if (flags & 1) {		// leaf
        if (flags & 2) {	// dense
            block = new BtreeDLeafBlock(pPh, beginsAt, endsBy, false);
        } else {
            block = new BtreeSLeafBlock(pPh, beginsAt, endsBy, false);
        }
    } else {
        block = new BtreeIntBlock(pPh, beginsAt, endsBy, false);
    }
    return block;
}

