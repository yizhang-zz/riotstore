#pragma once
#include "common/common.h"
#include "btree/BatchBuffer.h"
//#include <set>

namespace Btree
{
class BatchBufferFWF : public BatchBuffer
{
public:
	BatchBufferFWF(u32 cap_, BTree *tree_);
	~BatchBufferFWF();
	void put(const Key_t &key, const Datum_t &datum);
	void flushAll();
	bool find(const Key_t &key, Datum_t &datum);
protected:
	Entry *cache;
};
}
