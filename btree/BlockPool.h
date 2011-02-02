#ifndef BLOCK_POOL_H
#define BLOCK_POOL_H

#include <boost/pool/pool.hpp>
#include "BtreeBlock.h"
#include "BtreeSparseBlock.h"
#include "BtreeDenseLeafBlock.h"

namespace Btree
{
    /*
     * This class maintains a pool for Block objects. Because the number of 
     * Block objects is limited by the number of pages in the page buffer 
     * (BufferManager), we can preallocate the pool with total size = max(block 
     * object size) * #pages in BufferManager. When constructing a block object 
     * from the pageHandle, which contains the index of the page in the 
     * BufferManager's pool, we'll just grab a piece of memory that corresponds 
     * to that page in BlockPool and use that for the new Block object.
     */
    class BlockPool
    {
    public:
        BlockPool(int size_);
        ~BlockPool();
        // create a new block
        Block *create(Block::Type t, PageHandle ph, Key_t beginsAt, Key_t endsBy);
        // create an existing block
        Block *get(PageHandle ph, Key_t beginsAt, Key_t endsBy);
    private:
        int size;
        size_t unit;
        char *pool;
        /*
        boost::pool<boost::default_user_allocator_malloc_free> dpool;//(sizeof(DenseLeafBlock));
        boost::pool<boost::default_user_allocator_malloc_free> spool;//(sizeof(SparseLeafBlock));
        boost::pool<boost::default_user_allocator_malloc_free> ipool;//(sizeof(InternalLeafBlock));
        */
    };
}
#endif
