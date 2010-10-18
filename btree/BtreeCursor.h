#ifndef BTREE_CURSOR_H
#define BTREE_CURSOR_H

#include "../common/common.h"

namespace Btree
{
class Block;

class Cursor
{
public:
    static const int MaxDepth = 20;

	BufferManager *buffer;
    Block *trace[MaxDepth];
    int indices[MaxDepth];
    int current; // current position in the trace
	Key_t key;   // the key searched/inserted using this cursor
	
    Cursor(BufferManager *buf):buffer(buf),current(-1)
	{
	}

    ~Cursor()
	{
		for (int i=current; i>=0; i--) {
			buffer->unpinPage(trace[i]->pageHandle);
			delete trace[i];
		}
	}

	void grow()
	{
		assert(current<MaxDepth);
		for (int i=current; i>=0; i--) {
			trace[i+1] = trace[i];
			indices[i+1] = indices[i];
		}
		current++;
	}
};
}
#endif
