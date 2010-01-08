#ifndef BTREE_CURSOR_H
#define BTREE_CURSOR_H

#include "../common/common.h"

class Btree;
class BtreeBlock;

class BtreeCursor
{
public:
    static const int MaxDepth = 20;
    Btree *tree;
    BtreeBlock *trace[MaxDepth];
    u16 indices[MaxDepth];
    int current;

    BtreeCursor(Btree *t);

    ~BtreeCursor();
};
#endif
