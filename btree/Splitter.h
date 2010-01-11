#ifndef SPLITTER_H
#define SPLITTER_H

#include "BtreeBlock.h"

/**
 * An abstract class that defines the interface of splitting an overflowing
 * Btree block. All concrete splitting algorithms should be implemented in a
 * subclass of this class.
 */
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

/**
 * A strategy that splits an overflowing Btree block in the middle position.
 */
class MSplitter : public Splitter
{
public:
    virtual BtreeBlock* split(BtreeBlock *orig, PageHandle *newHandle);
};

/**
 * A strategy that splits an overflowing Btree block at the boundary that is
 * closest to the middle position.
 */
class BSplitter : public Splitter
{
public:
    virtual BtreeBlock* split(BtreeBlock *orig, PageHandle *newHandle);

    /**
     * Constructs a splitter with fixed boundary. Each future split must occur
     * on multiples of the given boundary.
     * @param boundary The splitting boundary.
     */
    BSplitter(u16 boundary) { this->boundary = boundary; }

private:
    u16 boundary;
};

#endif
