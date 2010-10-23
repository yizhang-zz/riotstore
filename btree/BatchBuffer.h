#pragma once

#include "../common/common.h"

namespace Btree
{
class BTree;

enum BatchMethod {
	kNone,
	kFWF,
	kFWPF,
	kLRU,
	kLS,
	kRAND,
	kRANDCUT
};

class BatchBuffer
{
public:
	BatchBuffer(u32 cap_, BTree *tree_):capacity(cap_), size(0), tree(tree_)
	{
	}

	virtual ~BatchBuffer()
	{
	}

	virtual void put(const Key_t &key, const Datum_t &datum) = 0;
	virtual void flushAll() = 0;
	virtual bool find(const Key_t &key, Datum_t &datum) = 0;
protected:
	u32 capacity;
	u32 size;
	BTree *tree;
};
}
