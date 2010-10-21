#include "BatchBufferFWF.h"
#include "Btree.h"
#include <algorithm>

using namespace Btree;

struct EntryComp {
	bool operator() (const Entry &a, const Entry &b)
	{
		return a.key < b.key;
	}
} entryComp;

BatchBufferFWF::BatchBufferFWF(u32 size, BTree *tree) : BatchBuffer(size,tree)
{
	cache_ = new Entry[size];
	next_ = 0;
}

BatchBufferFWF::~BatchBufferFWF()
{
	flushAll();
	delete cache_;
}

void BatchBufferFWF::put(const Key_t &key, const Datum_t &datum)
{
	if (next_==size_) {
		std::sort(cache_, cache_+size_, entryComp);
		// put one by one
		//for (int i=0; i<size_; ++i)
		//	tree_->put(cache_[i].key, cache_[i].datum);
		tree_->put(cache_, size_);
		next_ = 0;
	}
	cache_[next_].key = key;
	cache_[next_].datum = datum;
	++next_;
	return;
}

void BatchBufferFWF::flushAll()
{
	std::sort(cache_, cache_+next_, entryComp);
	tree_->put(cache_, next_);
	next_ = 0;
}

bool BatchBufferFWF::find(const Key_t &key, Datum_t &datum)
{
	for (int i=0; i<next_; ++i)
		if (cache_[i].key == key) {
			datum = cache_[i].datum;
			return true;
		}
	return false;
}
