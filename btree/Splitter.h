#ifndef SPLITTER_H
#define SPLITTER_H

#include <gsl/gsl_errno.h>
#include "BtreeBlock.h"
#include "BlockPool.h"
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
        Splitter(bool useDenseLeaf_) : useDenseLeaf(useDenseLeaf_)
        {
        }

		/**
		 * Splits the given node, which has already overflown, into two. The
		 * original node is reused as the left node, and the right node is
		 * returned.
		 *
		 * @param orig The original node.
		 * @param new_block The new block.
		 * @param ph The page handle for the new node.
		 * @param new_image The location where the new node should be stored.
		 * @return 0 if orig's format should change, nonzero otherwise.
		 */
		virtual int split(BlockT<Value> *orig, BlockT<Value> **newBlock,
						  PageHandle newPh, BlockPool *pool) = 0;
		virtual Splitter<Value> *clone() = 0;
    protected:
        int splitTypes(BlockT<Value> *block, Key_t *keys, int size, int sp, Key_t spKey, Block::Type types[2]);
        //void splitTypes(BlockT<Value> *block, int sp, Key_t spKey,
        //        Block::Type &left, Block::Type &right);
		int splitHelper(BlockT<Value> *orig, BlockT<Value> **newBlock,
						PageHandle newPh, BlockPool *pool,
						int sp,	Key_t spKey, Key_t *keys, Value *values,
                        Block::Type types[2]);
        bool useDenseLeaf;
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
        MSplitter(bool useDenseLeaf) : Splitter<Value>(useDenseLeaf)
        {
        }

		int split(BlockT<Value> *orig, BlockT<Value> **newBlock,
                PageHandle newPh, BlockPool *pool);

		Splitter<Value> *clone()
		{
			return new MSplitter<Value>(this->useDenseLeaf);
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
        /**
         * Constructs a splitter with fixed boundary. Each future split must 
         * occur on multiples of the given boundary.
         * @param boundary The splitting boundary.
		 */
        BSplitter(bool useDenseLeaf) : Splitter<Value>(useDenseLeaf)
        {
            if (useDenseLeaf)
                boundary = config->denseLeafCapacity;
            else
                boundary = config->sparseLeafCapacity;
        }

		int split(BlockT<Value> *orig, BlockT<Value> **newBlock,
                PageHandle newPh, BlockPool *pool);

		Splitter<Value> *clone()
		{
			return new BSplitter<Value>(this->useDenseLeaf);
		}

    private:
		int boundary;
    };


    template<class Value>
    class RSplitter : public Splitter<Value>
    {
    public:
        RSplitter(bool useDenseLeaf) : Splitter<Value>(useDenseLeaf) {}
		int split(BlockT<Value> *orig, BlockT<Value> **newBlock,
                PageHandle newPh, BlockPool *pool);
		Splitter<Value> *clone()
		{
			return new RSplitter<Value>(this->useDenseLeaf);
		}
    };

    template<class Value>
    class SSplitter : public Splitter<Value>
    {
    public:
        SSplitter(bool useDenseLeaf) : Splitter<Value>(useDenseLeaf)
		{
			gsl_set_error_handler(&riot_handler);
		}

		Splitter<Value> *clone()
		{
			return new SSplitter<Value>(this->useDenseLeaf);
		}

		int split(BlockT<Value> *orig, BlockT<Value> **newBlock,
                PageHandle newPh, BlockPool *pool);
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
		int split(BlockT<Value> *orig, BlockT<Value> **newBlock,
                PageHandle newPh, BlockPool *pool);

        TSplitter(double th, bool useDenseLeaf) : Splitter<Value>(useDenseLeaf)
                                                  ,threshold(th)
        {
        }

		Splitter<Value> *clone()
		{
			return new TSplitter<Value>(threshold, this->useDenseLeaf);
		}
    private:
		double threshold;
    };

    template<class Value>
    class SplitterFactory
    {
    public:
		static Splitter<Value> *createSplitter(char type, bool useDenseLeaf)
		{
			switch(type) {
			case 'A':
                return new BSplitter<Value>(useDenseLeaf);
			case 'M':
				return new MSplitter<Value>(useDenseLeaf);
			case 'R':
				return new RSplitter<Value>(useDenseLeaf);
			case 'S':
				return new SSplitter<Value>(useDenseLeaf);
			case 'T':
				return new TSplitter<Value>(config->TSplitterThreshold,useDenseLeaf);
			default:
				Error("Can't create unknown splitter %c", type);
				return NULL;
			}
		}
    };
}
#endif
