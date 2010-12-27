#ifndef SPLITTER_H
#define SPLITTER_H

#include <gsl/gsl_errno.h>
#include "BtreeBlock.h"
#include "common/Config.h"

void riot_handler(const char *reason, const char *file, int line, int gsl_errno);

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
		virtual int split(BlockT<Value> **orig, BlockT<Value> **newBlock,
						  PageHandle newPh) = 0;
		virtual Splitter<Value> *clone() = 0;
    protected:
		int splitHelper(BlockT<Value> **orig, BlockT<Value> **newBlock,
						PageHandle newPh,
						int sp,	Key_t spKey, Key_t *keys, Value *values);
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
		int split(BlockT<Value> **orig, BlockT<Value> **newBlock,
				  PageHandle newPh);
		Splitter<Value> *clone()
		{
			return new MSplitter<Value>;
		}
    };

    /**
     * A strategy that splits an overflowing Btree block at the boundary that is
     * closest to the middle position.
     */
    template<class Value>
    class BSplitter : public Splitter<Value>
    {
    public:
		int split(BlockT<Value> **orig, BlockT<Value> **newBlock,
				  PageHandle newPh);

		/**
		 * Constructs a splitter with fixed boundary. Each future split must occur
		 * on multiples of the given boundary.
		 * @param boundary The splitting boundary.
		 */
		BSplitter(int b):boundary(b) { }

		Splitter<Value> *clone()
		{
			return new BSplitter<Value>(boundary);
		}

    private:
		int boundary;
    };


    template<class Value>
    class RSplitter : public Splitter<Value>
    {
    public:
		int split(BlockT<Value> **orig, BlockT<Value> **newBlock,
				  PageHandle newPh);
		Splitter<Value> *clone()
		{
			return new RSplitter<Value>;
		}
    };

    template<class Value>
    class SSplitter : public Splitter<Value>
    {
    public:
		SSplitter()
		{
			gsl_set_error_handler(&riot_handler);
		}

		Splitter<Value> *clone()
		{
			return new SSplitter<Value>;
		}

		int split(BlockT<Value> **orig, BlockT<Value> **newBlock,
				  PageHandle newPh);
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
		int split(BlockT<Value> **orig, BlockT<Value> **newBlock,
				  PageHandle newPh);

		TSplitter(double th) : threshold(th) { }

		Splitter<Value> *clone()
		{
			return new TSplitter<Value>(threshold);
		}
    private:
		double threshold;
    };

    template<class Value>
    class SplitterFactory
    {
    public:
		static Splitter<Value> *createSplitter(char type)
		{
			switch(type) {
			case 'B':
				return new BSplitter<Value>(config->BSplitterBoundary());
			case 'M':
				return new MSplitter<Value>;
			case 'R':
				return new RSplitter<Value>;
			case 'S':
				return new SSplitter<Value>;
			case 'T':
				return new TSplitter<Value>(config->TSplitterThreshold);
			default:
				Error("Can't create unknown splitter");
				return NULL;
			}
		}
    };
}
#endif
