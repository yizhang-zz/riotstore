#include "BatchBuffer.h"
#include "Btree.h"

using namespace Btree;
using namespace std;

const BatchBuffer::CompEntry BatchBuffer::compEntry = BatchBuffer::CompEntry();

bool BatchBuffer::find(const Key_t &key, Datum_t &datum) 
{
	EntrySet::iterator it = entries.find(Entry(key,0));
	if (it == entries.end())
		return false;
	datum = it->datum;
	return true;
}

void BatchBuffer::flushAll() 
{
	tree->put(entries.begin(), entries.end());
	entries.clear();
	size = 0;
}

void BatchBuffer::print()
{
	ostream_iterator<Entry> output(cout, " ");
	copy(entries.begin(), entries.end(), output);
	cout<<endl;
}
