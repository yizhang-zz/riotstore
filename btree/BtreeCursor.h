#ifndef BTREE_CURSOR_H
#define BTREE_CURSOR_H

#include "../common/common.h"

namespace Btree
{
class BTree;
class Block;

class Cursor
{
public:
    static const int MaxDepth = 20;
    BTree *tree;
    Block *trace[MaxDepth];
    int indices[MaxDepth];
    int current;

    Cursor(BTree *t);

    ~Cursor();
};
}
#endif
