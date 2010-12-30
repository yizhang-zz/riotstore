#ifndef BLOCK_POOL_H
#define BLOCK_POOL_H

#include <boost/pool/pool.hpp>
#include "BtreeBlock.h"
#include "BtreeSparseBlock.h"
#include "BtreeDenseLeafBlock.h"

namespace Btree
{
    class BlockPool
    {
    public:
        BlockPool();
        ~BlockPool();
        // create a new block
        Block *create(Block::Type t, PageHandle ph, Key_t beginsAt, Key_t endsBy);
        // create an existing block
        Block *get(PageHandle ph, Key_t beginsAt, Key_t endsBy);
    private:
        boost::pool<boost::default_user_allocator_malloc_free> dpool;//(sizeof(DenseLeafBlock));
        boost::pool<boost::default_user_allocator_malloc_free> spool;//(sizeof(SparseLeafBlock));
        boost::pool<boost::default_user_allocator_malloc_free> ipool;//(sizeof(InternalLeafBlock));
    };
}
#endif
