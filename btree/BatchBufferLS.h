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
				, tree_), knownCount(0)
		{
			nKeep = 1+config->batchKeepPidCount;
			pids.reserve(nKeep);
			pids.push_back(maxPid);  // serve as sentinel
			lst.reserve(nKeep);
		}

		PageId computePids()
		{
			using namespace std;

			// lst will contain the running top K+1 pids as we combine the
			// existing pids and newly computed pids. We start by copying
			// everything in pids into lst.
			lst = pids;
			assert(lst.rbegin()->lower == MAX_KEY);
			lst.pop_back();  // the last one is the sentinel
			make_heap(lst.begin(), lst.end(), compCountR);

			EntrySet::const_iterator eit = entries.begin(),
				eit_end = entries.end();
			typename PidSet::const_iterator rit = pids.begin();
			while (eit != eit_end) {
				if (eit->key < rit->lower) {
					//if (!needsComputePids())
					//	break;
					PageId npid;
					tree->locate(eit->key, npid);
					// find all records going into npid
					while (eit != eit_end && eit->key < npid.upper) {
						++eit;
						++npid.count;
					}
					// insert the new pid
					if (lst.size() < nKeep) {
						lst.push_back(npid);
						push_heap(lst.begin(), lst.end(), compCountR);
					}
					else if (npid.count > lst.begin()->count) {
						pop_heap(lst.begin(), lst.end(), compCountR);
						*lst.rbegin() = npid;
						push_heap(lst.begin(), lst.end(), compCountR);
					}
				}
				else {
					// Since *rit is up to date, we can safely advance eit
					// to the end of *rit's range, and rit to the next
					eit = entries.lower_bound(rit->upper, compEntry);
					++rit; // safe because of the sentinel
				}
			}

			typename PidSet::iterator it = max_element(lst.begin(), lst.end(), compCount);
			PageId ret = *it;
			pids.clear();
			if (it != lst.begin())
				pids.insert(pids.end(), lst.begin(), it);
			pids.insert(pids.end(), ++it, lst.end());
			pids.push_back(maxPid);
			sort(pids.begin(), pids.end(), comp);
			return ret;
		}

		/*
		bool needsComputePids()
		{
			PidSetByCount &byCount = pids->get<1>();
			typename PidSetByCount::iterator it = byCount.begin();
			u16 x = it->count;
			return !(byCount.size() != 0 && x >= (++it)->count + (size-knownCount)); 
		}
		*/

		void put(const Key_t &key, const Datum_t &datum)
		{
			using namespace std;
			// TODO: implement random tie breaking
			if (size == capacity) {
				// try shortcut first: if first.count-second.count >= #records whose membership is unknown yet, 
				// then the first will be chosen no matter what. If random tie breaking is required, then the
				// above condition should be changed to >
				//if (needsComputePids())
				//	computePids();

				// evict the pid with largest count
				PageId longest = computePids();
				EntrySet::iterator start = entries.lower_bound(longest.lower, compEntry);
				EntrySet::iterator stop  = entries.lower_bound(longest.upper, compEntry);
				tree->put(start, stop);
				entries.erase(start, stop);
				size -= longest.count;
				knownCount -= longest.count;
			}

			entries.insert(Entry(key, datum));
			++size;
			// look up the current pid set
			typename PidSet::iterator it = 
				upper_bound(pids.begin(), pids.end(), key, comp);
			if (it != pids.begin() && --it != pids.end() &&
					(it->lower <= key && it->upper > key)) {
				it->count++;
				++knownCount;
			}
		}

		void flushAll()
		{
			BatchBuffer::flushAll();
			knownCount = 0;
			pids.clear();
			pids.push_back(maxPid);  // serve as sentinel
		}

		void print()
		{
			// first print all records and then the pid table
			using namespace std;
			BatchBuffer::print();
			ostream_iterator<PageId> output(cout, "\n");
			copy(pids.begin(), pids.end(), output);
			cout<<endl;
		}
private:
		PidSet pids;
		PidSet lst;
		int knownCount;
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
