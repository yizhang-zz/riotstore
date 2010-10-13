#ifndef SPLITTER_H
#define SPLITTER_H

#include "BtreeBlock.h"

namespace Btree
{
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
     * @param newPh The page handle where the new node should be stored.
     * @return The new node on the right after split.
     */
    virtual void split(Block **orig, Block **new_block, char *new_image) = 0;
};

/**
 * A strategy that splits an overflowing Btree block in the middle position.
 */
class MSplitter : public Splitter
{
public:
    void split(Block **orig, Block **new_block, char *new_image);
    //Block* split(Block *orig, char *new_image);
};

/**
 * A strategy that splits an overflowing Btree block at the boundary that is
 * closest to the middle position.
 */
class BSplitter : public Splitter
{
public:
    Block* split(Block *orig, PageHandle newPh);

    /**
     * Constructs a splitter with fixed boundary. Each future split must occur
     * on multiples of the given boundary.
     * @param boundary The splitting boundary.
     */
    BSplitter(u16 b):boundary(b) { }

private:
    u16 boundary;
};

/**
 * A strategy that splits an overflowing Btree block at the boundary that is
 * closest to the middle position.
 */
class RSplitter : public Splitter
{
public:
    Block* split(Block *orig, PageHandle newPh);
    // RSplitter(u16 boundary) { this->boundary = boundary; }
};

class SSplitter : public Splitter
{
public:
  /**
   * Split to maximize the S metric.
   */
    Block* split(Block *orig, PageHandle newPh);
private:
    double sValue(int b1, int b2, int d1, int d2);
};

class TSplitter : public Splitter
{
public:
  /**
   * If any of the two child nodes after split has a density greater
   * than the given threshold, then split so as to achieve the best
   * density; otherwise fallback to the M scheme.
   */
  Block* split(Block *orig, PageHandle newPh);
  TSplitter(double threshold) { this->threshold = threshold; }
  private:
  double threshold;
};
}
#endif
