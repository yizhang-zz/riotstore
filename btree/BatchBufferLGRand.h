#pragma once

#include "BatchBuffer.h"

namespace Btree
{
	template<class PageId>
	class BatchBufferLGRand: public BatchBuffer
	{
    private:
        gsl_rng *rng;

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
				int selected = gsl_rng_uniform_int(rng, size);
				int i;
				for (it = entries.begin(), i = 0; i < selected; ++it, ++i)
					;
				PageId spid;
				tree->locate(it->key, spid);
				it = entries.lower_bound(spid.lower, compEntry);
				// use a sentinel to bound the search
				typename EntrySet::const_iterator sentinel =
					entries.insert(entries.end(), Entry(MAX_KEY, 0));
				while (it->key < spid.upper) {
					++spid.count;
					++it;
				}
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
		}

	};
}
