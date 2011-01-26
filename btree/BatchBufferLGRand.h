#pragma once

#include "BatchBuffer.h"

namespace Btree
{
	template<class PageId>
	class BatchBufferLGRand: public BatchBuffer
	{
    private:
        gsl_rng *rng;
        Key_t selected;

	public:

		BatchBufferLGRand(u32 cap_, BTree *tree_): BatchBuffer(cap_ , tree_)
		{
            rng = gsl_rng_alloc(gsl_rng_taus2);
		}

        ~BatchBufferLGRand()
        {
            gsl_rng_free(rng);
        }

		void put(const Key_t &key, const Datum_t &datum)
		{
			using namespace std;
			typename EntrySet::const_iterator it;
			if (size == capacity) {
				PageId spid;
				tree->locate(selected, spid);
				// use a sentinel to bound the search
				typename EntrySet::const_iterator sentinel =
					entries.insert(entries.end(), Entry(MAX_KEY, 0));
				typename EntrySet::iterator it =
                    entries.lower_bound(spid.lower, compEntry);
				for (; it->key < spid.upper; ++it)
					++spid.count;
				entries.erase(sentinel);

				int group_lb = floorLog2(spid.count);  // group's size lower bound

				int count;
				for (it = entries.begin(); it != entries.end(); ) {
					PageId pid;
					tree->locate(it->key, pid);
					typename EntrySet::const_iterator git = it;
					for (++git, count=1; git != entries.end() && git->key < pid.upper; ++git, ++count)
						;
					if (group_lb == floorLog2(count)) {// same group
						tree->put(it, git);
						size -= count;
						entries.erase(it, git);
					}
					it = git;
				}
			}

			entries.insert(Entry(key, datum));
			++size;
            // reservoir sampling
            if (gsl_rng_uniform(rng) < 1.0/size)
                selected = key;
		}

	};
}
