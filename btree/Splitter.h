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
template<class Value>
class Splitter
{
public:
    /**
     * Splits the given node, which has already overflown, into two. The
     * original node is reused as the left node, and the right node is
     * returned.
     *
     * @param orig The original node.
     * @param new_block The new block.
     * @param ph The page handle for the new node.
     * @param new_image The location where the new node should be stored.
     * @return 0 if orig's format is not changed, nonzero otherwise.
     */
    virtual int split(BlockT<Value> **orig, BlockT<Value> **new_block,
					   PageHandle ph, char *new_image) = 0;
};

typedef Splitter<PID_t> InternalSplitter;
typedef Splitter<Datum_t> LeafSplitter;

/**
 * A strategy that splits an overflowing Btree block in the middle position.
 */
template<class Value>
class MSplitter : public Splitter<Value>
{
public:
    int split(BlockT<Value> **orig, BlockT<Value> **new_block,
			   PageHandle ph, char *new_image);
};

/**
 * A strategy that splits an overflowing Btree block at the boundary that is
 * closest to the middle position.
 */
template<class Value>
class BSplitter : public Splitter<Value>
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
template<class Value>
class RSplitter : public Splitter<Value>
{
public:
    Block* split(Block *orig, PageHandle newPh);
    // RSplitter(u16 boundary) { this->boundary = boundary; }
};

template<class Value>
class SSplitter : public Splitter<Value>
{
public:
  /**
   * Split to maximize the S metric.
   */
    Block* split(Block *orig, PageHandle newPh);
private:
    double sValue(int b1, int b2, int d1, int d2);
};

template<class Value>
class TSplitter : public Splitter<Value>
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
