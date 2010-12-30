#include "BlockPool.h"
#include "BtreeDenseLeafBlock.h"
#include "BtreeSparseBlock.h"

using namespace Btree;

BlockPool::BlockPool():
    dpool(sizeof(DenseLeafBlock)),
    spool(sizeof(SparseLeafBlock)),
    ipool(sizeof(InternalBlock))
{
}

BlockPool::~BlockPool()
{
}

Block *BlockPool::create(Block::Type t, PageHandle ph, Key_t beginsAt, Key_t endsBy)
{
    switch (t) {
    case Block::kInternal:
        return new (ipool.malloc()) InternalBlock(ph, beginsAt, endsBy, true);
    case Block::kDenseLeaf:
        return new (dpool.malloc()) DenseLeafBlock(ph, beginsAt, endsBy, true);
    case Block::kSparseLeaf:
        return new (spool.malloc()) SparseLeafBlock(ph, beginsAt, endsBy, true);
    default:
        return NULL;
    }
}

Block *BlockPool::get(PageHandle ph, Key_t beginsAt, Key_t endsBy)
{
	switch (*ph->getImage()) {
    case Block::kInternal:
        return new (ipool.malloc()) InternalBlock(ph, beginsAt, endsBy, false);
    case Block::kDenseLeaf:
        return new (dpool.malloc()) DenseLeafBlock(ph, beginsAt, endsBy, false);
    case Block::kSparseLeaf:
        return new (spool.malloc()) SparseLeafBlock(ph, beginsAt, endsBy, false);
	default:
		return NULL;
	}
}
