#ifndef CACHE_H
#define CACHE_H

#include "../block/common.h"

class Cache {
public:
    uint32_t block_count; /* total # of blocks in the disk file */
    uint32_t capacity; /* # of blocks that can be loaded in memory */
    uint32_t size; /* current # of blocks in memory */

    /* Need data structures for lookups based on BlockNo and eviction
    policy (e.g. last access time). */

    Cache(const char* file);
    ~Cache();

    int loadBlock(BlockNo n, Block **b);
    /*int releaseBlock(Block *b);*/

    /* We allocate space ourselves, because data can be shared by multiple
    consumers. */
    int writeBlock(BlockNo n);
    int readBlock(BlockNo n, char **p);

    /* Choose the block to be evicted if size==capacity and a new block
    is requested. */
    int evict();
};
#endif
