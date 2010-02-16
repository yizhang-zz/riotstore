#ifndef BTREE_BLOCK_PACKER_H
#define BTREE_BLOCK_PACKER_H

#include "../common/common.h"
#include "../lower/BufferManager.h"
#include <apr-1/apr_pools.h>
namespace Btree
{

class BtreePagePacker : public ::PagePacker
{
public:
    void pack(void *unpacked, void *packed);
    void unpack(void *packed, void *&unpacked);
    void destroyUnpacked(void *&unpacked);
    static apr_pool_t *pool;
};

}
#endif
