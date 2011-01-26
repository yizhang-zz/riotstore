#pragma once

#include "BatchBuffer.h"

namespace Btree
{
	template<class PageId>
	class BatchBufferLSRand: public BatchBuffer
	{
    private:
        gsl_rng *rng;
        Key_t selected;

	public:
		BatchBufferLSRand(u32 cap_, BTree *tree_): BatchBuffer(cap_, tree_)
		{
            rng = gsl_rng_alloc(gsl_rng_taus2);
		}

        ~BatchBufferLSRand()
        {
            gsl_rng_free(rng);
        }

		void put(const Key_t &key, const Datum_t &datum)
		{
			using namespace std;
			if (size == capacity) {
                /* sampling by walking through the set is too expensive!
				int index = gsl_rng_uniform_int(rng, size);
				EntrySet::iterator it = entries.begin(),
					it_end = entries.end();
				for (int i=0; i<index; ++i,++it)
					;
				PageId pid;
				tree->locate(it->key, pid);
                */
				PageId pid;
				tree->locate(selected, pid);
				EntrySet::iterator begin = entries.lower_bound(pid.lower, compEntry);
				EntrySet::iterator end = entries.lower_bound(pid.upper, compEntry);
				int ret = tree->put(begin, end);
				//cout<<"put "<<ret<<" records into pid ["<<pid.lower<<","<<pid.upper<<")"<<endl;
				size -= ret;
				entries.erase(begin, end);
			}

			entries.insert(Entry(key, datum));
			++size;
            // reservoir sampling
            if (gsl_rng_uniform(rng) < 1.0/size)
                selected = key;
		}
	};
}
