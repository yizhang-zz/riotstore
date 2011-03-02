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
	class BatchBufferLRU: public BatchBuffer
	{
	public:

		typedef std::vector<PageId> PidList;
        Entry maxEntry;
        Entry *entriesArray;
        u64 *accessTime;
        int freehead;
        u64 ts; // pseudo-timestamp

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

        void add(Entry e)
        {
            assert(freehead != -1);
            int next = *(int*)(entriesArray+freehead);
            entriesArray[freehead] = e;
            accessTime[freehead] = ts++;
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
        // size of pageId = lower+upper+count=8+8+4
        // size of entry  = key+datum=8+8
		BatchBufferLRU(u32 cap_, BTree *tree_): BatchBuffer(
				(cap_-(8+8+4)*config->batchKeepPidCount/(8+8))/(1.0+2*ceilingLog2(cap_)/8.0/(8+8))
				, tree_), maxEntry(MAX_KEY, 0)
		{
            using namespace std;
            cerr<<"Effective capacity = "<<capacity<<endl;
			nKeep = 1+config->batchKeepPidCount;
			pids = new PidList;
            pids->reserve(nKeep);
            pids->push_back(maxPid); // sentinel
            entries.insert(&maxEntry); // sentinel
            ts = 0;
            entriesArray = new Entry[capacity];
            accessTime = new u64[capacity];
            initFreeList();
		}

        ~BatchBufferLRU()
        {
            // remove the sentinel
            entries.erase(MAX_KEY);
            tree->put(Iterator(entries.begin()), Iterator(entries.end()));
            delete[] entriesArray;
            delete[] accessTime;
            delete pids;
        }

		PageId getLRU()
		{
			using namespace std;
            PageId ret;
            PageId lru_page;
            // TODO: count is 32 bit but timestamp is 64 bit!
            lru_page.count = UINT32_MAX;
            EntryPtrSet::iterator it = entries.begin();
            typename PidList::iterator rit = pids->begin(), lit = pids->end();
            while ((*it)->key < MAX_KEY) {
                if ((*it)->key < rit->lower) {
                    PageId pid;
                    pid.count = 0; // most recent reference
                    tree->locate((*it)->key, pid);
                    for (; (*it)->key < pid.upper; ++it) {
                        u64 temp = accessTime[*it - entriesArray];
                        if ((pid.count) < temp)
                            pid.count = temp;
                    }
                    if (lru_page.count > pid.count) {
                        lru_page = pid;
                        if (pids->size() < nKeep) {
                            lit = pids->insert(rit, pid);
                            ++(rit=lit);
                        }
                    }
                    else if (pids->size() < nKeep) {
                        rit = pids->insert(rit, pid);
                        ++rit;
                    }
                }
                else {
                    if (lru_page.count > rit->count) {
                        lru_page = *rit;
                        lit = rit;
                    }
					// Since *rit is up to date, we can safely advance eit
					// to the end of *rit's range, and rit to the next
					it = entries.lower_bound(rit->upper);
					++rit; // safe because of the sentinel
				}
            }

            if (lit != pids->end()) {
                pids->erase(lit);
                // strictly speaking we need to cache one more pid to make
                // sure the total number of pids is nKeep
            }
            return lru_page;
		}

		void put(const Key_t &key, const Datum_t &datum)
		{
            using namespace std;
            if (size == capacity) {
                PageId pid = getLRU();
                EntryPtrSet::iterator begin = entries.lower_bound(pid.lower);
                EntryPtrSet::iterator end = entries.lower_bound(pid.upper);
                int ret = tree->put(Iterator(begin), Iterator(end));
                size -= ret;
                remove(begin, end);
            }

            add(Entry(key, datum));
            ++size;
			typename PidList::iterator it = 
				upper_bound(pids->begin(), pids->end(), key, comp);
			if (it != pids->begin() && (--it)->contains(key)) {
                it->count = ts; // most recent reference
			}
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
            entries.erase(MAX_KEY); // remove sentinel
            tree->put(Iterator(entries.begin()), Iterator(entries.end()));
            entries.clear();
            initFreeList();
            entries.insert(&maxEntry); // add back sentinel
            size = 0;
			pids->clear();
			pids->push_back(maxPid);  // serve as sentinel
        }
protected:
		PidList *pids;
		unsigned nKeep;
		static CompPidCount<PageId> compCount;
		static CompPidCountR<PageId> compCountR;
		static CompPid<PageId> comp;
		static PageId maxPid;
	};

	template<class PageId>
	CompPidCount<PageId> BatchBufferLRU<PageId>::compCount;
	template<class PageId>
	CompPidCountR<PageId> BatchBufferLRU<PageId>::compCountR;
	template<class PageId>
	CompPid<PageId> BatchBufferLRU<PageId>::comp;
	template<class PageId>
	PageId BatchBufferLRU<PageId>::maxPid(MAX_KEY, MAX_KEY, 0);
}
