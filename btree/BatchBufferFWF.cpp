#include "BatchBufferFWF.h"
#include "Btree.h"

using namespace Btree;

BatchBufferFWF::BatchBufferFWF(u32 cap_, BTree *tree_): BatchBuffer(cap_,tree_)
{
}

BatchBufferFWF::~BatchBufferFWF()
{
    tree->put(entries.begin(), entries.end());
}

void BatchBufferFWF::put(const Key_t &key, const Datum_t &datum)
{
	if (size==capacity)
		flushAll();
	entries.insert(Entry(key, datum));
	++size;
	return;
}
