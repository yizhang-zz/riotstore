#ifdef USE_BATCH_BUFFER
#include "BatchBufferFWF.h"
#include "Btree.h"
#include <algorithm>
#include <iostream>

using namespace Btree;
using namespace std;

struct EntryComp {
	bool operator() (const Entry &a, const Entry &b)
	{
		return a.key < b.key;
	}
} entryComp;

BatchBufferFWF::BatchBufferFWF(u32 cap_, BTree *tree_) : BatchBuffer(cap_,tree_)
{
}

BatchBufferFWF::~BatchBufferFWF()
{
	flushAll();
}

void BatchBufferFWF::put(const Key_t &key, const Datum_t &datum)
{
	if (size==capacity)
		flushAll();
	cache.insert(Entry(key, datum));
	++size;
	return;
}

void BatchBufferFWF::flushAll()
{
	tree->put(cache.begin(), cache.end());
	cache.clear();
	size = 0;
}

bool BatchBufferFWF::find(const Key_t &key, Datum_t &datum)
{
	std::set<Entry>::iterator it = cache.find(Entry(key, 0));
	if (it == cache.end())
		return false;
	datum = it->datum;
	return true;
}

void BatchBufferFWF::print()
{
	ostream_iterator<Entry> output(cout, " ");
	copy(cache.begin(), cache.end(), output);
	cout<<endl;
}
#endif
