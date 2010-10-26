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
	cache = new Entry[capacity];
}

BatchBufferFWF::~BatchBufferFWF()
{
	flushAll();
	delete cache;
}

void BatchBufferFWF::put(const Key_t &key, const Datum_t &datum)
{
	if (size==capacity)
		flushAll();
	cache[size].key = key;
	cache[size].datum = datum;
	++size;
	return;
}

void BatchBufferFWF::flushAll()
{
	std::sort(cache, cache+size, entryComp);
	tree->put(cache, cache+size);
	size = 0;
}

bool BatchBufferFWF::find(const Key_t &key, Datum_t &datum)
{
	for (int i=0; i<size; ++i)
		if (cache[i].key == key) {
			datum = cache[i].datum;
			return true;
		}
	return false;
}

void BatchBufferFWF::print()
{
	for (int i=0; i<size; ++i)
		cout<<cache[i].key<<" ";
	cout<<endl;
}
#endif
