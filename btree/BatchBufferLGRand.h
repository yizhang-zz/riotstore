#pragma once

#include "BatchBuffer.h"

namespace Btree
{
	template<class PageId>
	class BatchBufferLGRand: public BatchBuffer
	{
    private:
        gsl_rng *rng;
        Entry *entriesArray;
        int freehead;
        EntryPtrSet entries;

        void remove(EntryPtrSet::iterator begin, EntryPtrSet::iterator end)
        {
            using namespace std;
            for (EntryPtrSet::iterator it = begin; it != end; ++it) {
                *((int*) *it) =  freehead;
                freehead = *it - entriesArray;
            }
            entries.erase(begin, end);
        }

        void add(const Entry &e)
        {
            using namespace std;
            int next = *(int*)(entriesArray+freehead);
            //cerr<<next<<" "<<endl;
            entriesArray[freehead] = e;
            entries.insert(entriesArray+freehead);
            freehead = next;
        }

        void initFreeList()
        {
            freehead = 0;
            int *backptr = &freehead;
            for (u32 i=0; i<capacity; ++i) {
                *backptr = i;
                backptr = (int*) (entriesArray+i);
            }
            *backptr = -1;
        }

	public:

		BatchBufferLGRand(u32 cap_, BTree *tree_):
            BatchBuffer(cap_/(1.0+ceilingLog2(cap_)/8/sizeof(Entry)), tree_)
		{
            rng = gsl_rng_alloc(gsl_rng_taus2);
            entriesArray = new Entry[capacity];
            initFreeList();
		}

        ~BatchBufferLGRand()
        {
            tree->put(Iterator(entries.begin()), Iterator(entries.end()));
            delete[] entriesArray;
            gsl_rng_free(rng);
        }

		void put(const Key_t &key, const Datum_t &datum)
		{
			using namespace std;
			typename EntrySet::const_iterator it;
			if (size == capacity) {
                // insert a sentinel to the end of the set so that the
                // != end() test isn't necessary below
                Entry sentinel(MAX_KEY, 0);
                EntryPtrSet::iterator it_sentinel = entries.insert(
                        entries.end(), &sentinel);

				int index = gsl_rng_uniform_int(rng, size);
                PageId spid;
                tree->locate(entriesArray[index].key, spid);
				typename EntryPtrSet::iterator it =
                    entries.lower_bound(spid.lower);
				for (; (*it)->key < spid.upper; ++it)
					++spid.count;

				int group_lb = floorLog2(spid.count);  // group's size lower bound
				int count;
				for (it = entries.begin(); (*it)->key < MAX_KEY; ) {
					PageId pid;
					tree->locate((*it)->key, pid);
					typename EntryPtrSet::const_iterator git = it;
					for (++it, count=1; (*it)->key < pid.upper; ++it, ++count)
						;
					if (group_lb == floorLog2(count)) {// same group
						tree->put(Iterator(git), Iterator(it));
						size -= count;
                        remove(git, it);
					}
				}

                //cerr<<(*it)->key<<endl;
                // erase the sentinel
                entries.erase(it_sentinel);
			}

            //cerr<<"add "<<key<<endl;
			add(Entry(key, datum));
			++size;
		}

        bool find(const Key_t &key, Datum_t &datum)
        {
            EntryPtrSet::iterator it = entries.find(key);
            if (it == entries.end())
                return false;
            datum = (*it)->datum;
            return true;
        }

        void flushAll()
        {
            tree->put(Iterator(entries.begin()), Iterator(entries.end()));
            entries.clear();
            initFreeList();
            size = 0;
        }
	};
}
