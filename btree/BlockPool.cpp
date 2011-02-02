#include "BlockPool.h"
#include "BtreeDenseLeafBlock.h"
#include "BtreeSparseBlock.h"
#include <algorithm>

using namespace Btree;

BlockPool::BlockPool(int size_): size(size_)
/*    dpool(sizeof(DenseLeafBlock)),
    spool(sizeof(SparseLeafBlock)),
    ipool(sizeof(InternalBlock))
    */
{
    unit = std::max(sizeof(SparseLeafBlock), sizeof(DenseLeafBlock));
    unit = std::max(unit, sizeof(InternalBlock));
    pool = (char*) malloc(unit * size);
}

BlockPool::~BlockPool()
{
    free(pool);
}

Block *BlockPool::create(Block::Type t, PageHandle ph, Key_t beginsAt, Key_t endsBy)
{
    switch (t) {
    case Block::kInternal:
        return new (pool+unit*ph->getIndex()) InternalBlock(ph, beginsAt, endsBy, true);
    case Block::kDenseLeaf:
        return new (pool+unit*ph->getIndex()) DenseLeafBlock(ph, beginsAt, endsBy, true);
    case Block::kSparseLeaf:
        return new (pool+unit*ph->getIndex()) SparseLeafBlock(ph, beginsAt, endsBy, true);
    default:
        return NULL;
    }
}

Block *BlockPool::get(PageHandle ph, Key_t beginsAt, Key_t endsBy)
{
	switch (*ph->getImage()) {
    case Block::kInternal:
        return new (pool+unit*ph->getIndex()) InternalBlock(ph, beginsAt, endsBy, false);
    case Block::kDenseLeaf:
        return new (pool+unit*ph->getIndex()) DenseLeafBlock(ph, beginsAt, endsBy, false);
    case Block::kSparseLeaf:
        return new (pool+unit*ph->getIndex()) SparseLeafBlock(ph, beginsAt, endsBy, false);
	default:
		return NULL;
	}
}
