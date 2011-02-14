#pragma once

#include "BatchBuffer.h"
#include <iostream>
#include <iterator>
#include <algorithm>
#include <list>
#include <boost/pool/pool.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>

using namespace boost;
using namespace boost::multi_index;

namespace Btree
{
	template<class PageId>
	class BatchBufferLS: public BatchBuffer
	{
	public:

		typedef std::vector<PageId> PidSet;

		BatchBufferLS(u32 cap_, BTree *tree_): BatchBuffer(
				cap_-sizeof(PageId)*config->batchKeepPidCount/sizeof(Entry)
				, tree_)
		{
			nKeep = 1+config->batchKeepPidCount;
			pids = new PidSet(1, maxPid); // sentinel
            entries.insert(Entry(MAX_KEY, 0)); // sentinel
		}

        ~BatchBufferLS()
        {
            // remove the sentinel
            entries.erase(MAX_KEY);
            tree->put(entries.begin(), entries.end());
        }

		PageId computePids()
		{
			using namespace std;
#if 0
            // shortcut: if max among known pids >= last flushed page, then
            // directly flush that page and skip computing the unknown pids
			typename PidSet::iterator it = max_element(pids.begin(), pids.end(), compCount);
            if (it->count >= lastFlushCount && it->count > 0) {
                PageId ret = *it;
                pids.erase(it);
                return ret;
            }
#endif

			// lst will contain the running top K+1 pids as we combine the
			// existing pids and newly computed pids. We start by copying
			// everything in pids into lst, and then make a min-heap.
			PidSet *lst = new PidSet(*pids); // lst contains a maxPid
			make_heap(lst->begin(), lst->end(), compCountR);
            unsigned maxKnownPidSize = max_element(lst->begin(),
                    lst->end(), compCount)->count;

            unsigned knownCount = 0;
			EntrySet::const_iterator eit = entries.begin();
			typename PidSet::const_iterator rit = pids->begin();
			//while (eit->key < MAX_KEY) {
			while (maxKnownPidSize < size - knownCount) {
                assert(eit->key != MAX_KEY);
				if (eit->key < rit->lower) {
					PageId npid;
					tree->locate(eit->key, npid);
					// find all records going into npid
					while (eit->key < npid.upper) {
						++eit;
						++npid.count;
					}
                    knownCount += npid.count;
                    if (npid.count > maxKnownPidSize)
                        maxKnownPidSize = npid.count;

					// insert the new pid
                    if (lst->size() < nKeep) {
                        lst->push_back(npid);
						push_heap(lst->begin(), lst->end(), compCountR);
                    }
                    else if (npid.count > lst->begin()->count) {
                        // replace the heap top
						pop_heap(lst->begin(), lst->end(), compCountR);
						lst->back() = npid;
						push_heap(lst->begin(), lst->end(), compCountR);
					}
				}
				else {
					// Since *rit is up to date, we can safely advance eit
					// to the end of *rit's range, and rit to the next
					eit = entries.lower_bound(rit->upper);
					++rit; // safe because of the sentinel
                    knownCount += rit->count;
				}
			}

            // Largest page, to be flushed
			typename PidSet::iterator it = max_element(lst->begin(), lst->end(), compCount);
			PageId ret = *it;
            *it = lst->back(); // move last element to the empty slot
            if (lst->begin()->count)
                // top of min-heap is non-zero, so no maxPid in lst
                // replace it with a maxPid
                lst->back() = maxPid;
            else
                // top of min-heap is maxPid, just remove the last element
                lst->pop_back();
			sort(lst->begin(), lst->end());

            delete pids;
            pids = lst;
            lst = NULL;
            
			return ret;
		}

		void put(const Key_t &key, const Datum_t &datum)
		{
			using namespace std;
			if (size == capacity) {
                unsigned temp = 0;
                for (typename PidSet::iterator it=pids->begin(); it != pids->end();++it)
                    temp += it->count;
				// evict the pid with largest count
				PageId longest = computePids();
				EntrySet::iterator start = entries.lower_bound(longest.lower);
				EntrySet::iterator stop  = entries.lower_bound(longest.upper);
                tree->put(start, stop);
				entries.erase(start, stop);
				size -= longest.count;
			}

			entries.insert(Entry(key, datum));
			++size;
			// Look up the current pid set for key's pid. Because of the
            // sentinel, it won't be pids->end().
			typename PidSet::iterator it = 
				upper_bound(pids->begin(), pids->end(), key, comp);
			if (it != pids->begin() && (--it)->contains(key)) {
				it->count++;
			}
		}

		void flushAll()
		{
            entries.erase(MAX_KEY); // remove sentinel
			BatchBuffer::flushAll();
            entries.insert(Entry(MAX_KEY, 0));
			pids->clear();
			pids->push_back(maxPid);  // serve as sentinel
		}

		void print()
		{
			// first print all records and then the pid table
			using namespace std;
			//BatchBuffer::print();
			ostream_iterator<PageId> output(cout, "\n");
			copy(pids->begin(), pids->end(), output);
			cout<<endl;
		}

        void printRange(Key_t b, Key_t e)
        {
            using namespace std;
            int n = 0;
            EntrySet::iterator it = entries.lower_bound(b);
            while (it->key < e) {
                cerr<<it->key<<" ";
                ++n;
                ++it;
            }
            cerr<<"total="<<n<<endl;
        }

protected:
		PidSet *pids;
		PidSet *lst;
		unsigned nKeep;
		static CompPidCount<PageId> compCount;
		static CompPidCountR<PageId> compCountR;
		static CompPid<PageId> comp;
		static PageId maxPid;
	};

	template<class PageId>
	CompPidCount<PageId> BatchBufferLS<PageId>::compCount;
	template<class PageId>
	CompPidCountR<PageId> BatchBufferLS<PageId>::compCountR;
	template<class PageId>
	CompPid<PageId> BatchBufferLS<PageId>::comp;
	template<class PageId>
	PageId BatchBufferLS<PageId>::maxPid(MAX_KEY, MAX_KEY, 0);
}
