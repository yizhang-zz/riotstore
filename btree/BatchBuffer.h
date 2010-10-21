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
	BatchBuffer(u32 size, BTree *tree):size_(size), tree_(tree)
	{
	}

	virtual ~BatchBuffer()
	{
	}

	virtual void put(const Key_t &key, const Datum_t &datum) = 0;
	virtual void flushAll() = 0;
	virtual bool find(const Key_t &key, Datum_t &datum) = 0;
  protected:
	u32 size_;
	BTree *tree_;
};
}
