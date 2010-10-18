#include "BtreeCursor.h"
#include "Btree.h"
#include "BtreeBlock.h"

using namespace Btree;

Cursor::Cursor(BTree *t)
{
    tree = t;
    current = -1;
}

Cursor::~Cursor()
{
    for (int i=current; i>=0; i--) {
        tree->releasePage(trace[i]->pageHandle);
        delete trace[i];
    }
}
