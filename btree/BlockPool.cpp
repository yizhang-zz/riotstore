#include "BlockPool.h"
#include "BtreeDenseLeafBlock.h"
#include "BtreeSparseBlock.h"
#include <algorithm>

using namespace Btree;

BlockPool::BlockPool():
    dpool(std::max(std::max(sizeof(DenseLeafBlock), sizeof(SparseLeafBlock)), sizeof(InternalBlock)))
{
    unit = std::max(sizeof(SparseLeafBlock), sizeof(DenseLeafBlock));
    unit = std::max(unit, sizeof(InternalBlock));
    //pool = (char*) malloc(unit * size);
}

BlockPool::~BlockPool()
{
    //free(pool);
}

Block *BlockPool::create(Block::Type t, PageHandle ph, Key_t beginsAt, Key_t endsBy)
{
    switch (t) {
    case Block::kInternal:
        return new (dpool.malloc()) InternalBlock(ph, beginsAt, endsBy, true);
    case Block::kDenseLeaf:
        return new (dpool.malloc()) DenseLeafBlock(ph, beginsAt, endsBy, true);
    case Block::kSparseLeaf:
        return new (dpool.malloc()) SparseLeafBlock(ph, beginsAt, endsBy, true);
    default:
        return NULL;
    }
}

Block *BlockPool::get(PageHandle ph, Key_t beginsAt, Key_t endsBy)
{
	switch (*ph->getImage()) {
    case Block::kInternal:
        return new (dpool.malloc()) InternalBlock(ph, beginsAt, endsBy, false);
    case Block::kDenseLeaf:
        return new (dpool.malloc()) DenseLeafBlock(ph, beginsAt, endsBy, false);
    case Block::kSparseLeaf:
        return new (dpool.malloc()) SparseLeafBlock(ph, beginsAt, endsBy, false);
	default:
		return NULL;
	}
}
