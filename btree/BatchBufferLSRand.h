#pragma once

#include "BatchBuffer.h"

namespace Btree
{
	template<class PageId>
	class BatchBufferLSRand: public BatchBuffer
	{
	public:

		BatchBufferLSRand(u32 cap_, BTree *tree_): BatchBuffer(cap_, tree_)
		{
		}

		void put(const Key_t &key, const Datum_t &datum)
		{
			using namespace std;
			if (size == capacity) {
				int index = rand() % size;
				EntrySet::iterator it = entries.begin(),
					it_end = entries.end();
				for (int i=0; i<index; ++i,++it)
					;
				PageId pid;
				tree->locate(it->key, pid);
				EntrySet::iterator begin = entries.lower_bound(pid.lower, compEntry);
				EntrySet::iterator end = entries.lower_bound(pid.upper, compEntry);
				int ret = tree->put(begin, end);
				//cout<<"put "<<ret<<" records into pid ["<<pid.lower<<","<<pid.upper<<")"<<endl;
				size -= ret;
				entries.erase(begin, end);
			}

			entries.insert(Entry(key, datum));
			++size;
		}
	};
}
