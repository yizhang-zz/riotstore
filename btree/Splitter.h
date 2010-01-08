#ifndef SPLITTER_H
#define SPLITTER_H

#include "BtreeBlock.h"

class Splitter
{
public:
    /**
     * Splits the given node, which has already overflown, into two. The
     * original node is reused as the left node, and the right node is
     * returned.
     *
     * @param orig The original node.
     * @return The new node on the right after split.
     */
    virtual BtreeBlock* split(BtreeBlock *orig, PageHandle *newHandle) = 0;
};

class MSplitter : public Splitter
{
public:
    virtual BtreeBlock* split(BtreeBlock *orig, PageHandle *newHandle);
};

#endif
