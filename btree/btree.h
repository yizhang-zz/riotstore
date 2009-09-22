#ifndef BTREE_H
#define BTREE_H

#include "../block/common.h"
#include "../block/block.h"
#include "utils.h"

#define ROOT 1
#define INTERNAL 2
#define LEAF 4

#define INS_OVERFLOW 11
#define INS_NORMAL 12
#define INS_DENSE 13


class BTree {
    public:
        uint_32 leaf_count;
        int depth;
        BlockNo root_no;
        BlockNo first_leaf_no;
        Block *root;
        Block *first_leaf;


    public:
        BTree(int capacity, Key start, Key end);
        SearchResult search(Key key);

        int put(Key key, Datum data);
        int put(Key begin, Key end, Datum *data);
        int put(Key *key, Datum *data, int size);

        int get(Key key, Datum *datum);
        int get(Key begin, Key end, Datum *data);
        int get(Key *keys, Datum *data, int size);
};

#endif
