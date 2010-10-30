#include "BatchBufferFWF.h"

using namespace Btree;

void BatchBufferFWF::put(const Key_t &key, const Datum_t &datum)
{
	if (size==capacity)
		flushAll();
	entries.insert(Entry(key, datum));
	++size;
	return;
}
