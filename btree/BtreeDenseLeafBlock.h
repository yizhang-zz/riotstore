#ifndef BTREE_DENSE_LEAF_BLOCK_H
#define BTREE_DENSE_LEAF_BLOCK_H

#include "../common/common.h"
#include "BtreeBlock.h"

namespace Btree
{
class DenseLeafBlock : public Block
{
public:
	/* Header consists of the following:
	 * Size		Note
	 * 1		flag
	 * 1		reserved
	 * 2		number of non-zero entries stored
	 * 4		page ID of next leaf
	 * 2		head pointer (index in the circular buffer)
	 * 2		tail pointer (index in the circular buffer)
	 * 4		key of the head element 
	 *
	 * We only store a segment among the [lower, upper) range. To facilitate expanding
	 * the segment from both directions, we use a circular buffer to store the segment.
	 * When the head and tail coincide, we decide if the buffer is full or empty by
	 * checking the number of non-zero entries.
	 */
	const static int HeaderSize = 16;
	//static u16 capacity() { return _capacity; }
	static u16 capacity;

	u16 *headIndex;
	u16 *tailIndex;
	Key_t *headKey;

	DenseLeafBlock(BTree *tree, PageHandle ph, Key_t beginsAt, Key_t endsBy, bool create);
	int search(Key_t key, int &index) const;
	int get(Key_t key, Value &v) const;
	int get(int index, Key_t &key, Value &v) const;
	int put(Key_t key, const Value &v);

private:
	Datum_t *data;
	static u16 initCapacity();
	Key_t getTailKey() const
	{
		if (*tailIndex > *headIndex) {
			return *headKey+(*tailIndex-*headIndex);
		}
		else if (*tailIndex < *headIndex) {
			return *headKey+(capacity+*tailIndex-*headIndex);
		}
		else {
			if (*nEntries)
				return *headKey+capacity;
			else
				return *headKey;
		}
	}


};
}

#endif
