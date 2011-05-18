#ifndef BTREE_CURSOR_H
#define BTREE_CURSOR_H

#include "../common/common.h"
#include "BtreeSparseBlock.h"
#include "BtreeDenseLeafBlock.h"

namespace Btree
{
class Block;

class Cursor
{
public:
    static const int MaxDepth = 20;

	struct Level {
		Block *block;
		int index;
	};

	//BufferManager *buffer;
	Level levels[MaxDepth];
    //Block *trace[MaxDepth];
    //int indices[MaxDepth];
    int current; // current position in the trace
	Key_t key;   // the key searched/inserted using this cursor
	
    Cursor():current(-1)
	{
	}

    ~Cursor()
	{
        // blocks are all created by placement new operator
        // so should not call delete
		for (int i=current; i>=0; i--) {
			PageHandle ph = levels[i].block->pageHandle;
			ph->unpin();
			assert(ph->pinCount >= 0);
		}
	}

	void grow()
	{
		assert(current<MaxDepth);
		for (int i=current; i>=0; i--) {
			levels[i+1] = levels[i];
		}
		current++;
	}

	Level & operator[](int i) 
	{
		return levels[i];
	}

	const Level & operator[](int i) const
	{
		return levels[i];
	}
};
}
#endif
