#include "BtreeCursor.h"
#include "Btree.h"
#include "BtreeBlock.h"

BtreeCursor::BtreeCursor(Btree *t)
{
    tree = t;
    current = -1;
}

BtreeCursor::~BtreeCursor()
{
    for (int i=current; i>=0; i--) {
        tree->releasePage(trace[i]->getPageHandle());
    }
}
