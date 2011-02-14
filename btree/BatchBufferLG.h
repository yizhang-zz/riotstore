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
	class BatchBufferLG: public BatchBuffer
	{
	public:

		typedef std::vector<PageId> PidSet;

		struct Group
		{
			PidSet pids;
			int count;

			void clear()
			{
				count = 0;
				pids.clear();
			}

			bool operator<(const Group &g) { return count<g.count; }
		};

		BatchBufferLG(u32 cap_, BTree *tree_): BatchBuffer(
				cap_-sizeof(PageId)*config->batchKeepPidCount/sizeof(Entry)
				, tree_)
		{
			pids.reserve(1+config->batchKeepPidCount);
			pids.push_back(maxPid);  // serve as sentinel
			nGroups = floorLog2(capacity-1)+1;
			groups = new Group[nGroups];
		}

		~BatchBufferLG()
		{
            tree->put(entries.begin(), entries.end());
			delete[] groups;
		}

		int computeGroups()
		{
			using namespace std;

			int maxIndex = 0;
			int maxCount = 0;
			typename PidSet::const_iterator rit = pids.begin();
			EntrySet::const_iterator eit = entries.begin(),
				eit_end = entries.end();
			while (eit != eit_end) {
				if (eit->key < rit->lower) {
					PageId npid;
					tree->locate(eit->key, npid);
					// find all records going into npid
					while (eit != eit_end && eit->key < npid.upper) {
						++eit;
						++npid.count;
					}
					// find associated group
					int iGroup = floorLog2(npid.count);
					groups[iGroup].count += npid.count;
					groups[iGroup].pids.push_back(npid);
					if (groups[iGroup].count > maxCount) {
						maxIndex = iGroup;
						maxCount = groups[iGroup].count;
					}
				}
				else {
					int iGroup = floorLog2(rit->count);
					groups[iGroup].count += rit->count;
					groups[iGroup].pids.push_back(*rit);
					if (groups[iGroup].count > maxCount) {
						maxIndex = iGroup;
						maxCount = groups[iGroup].count;
					}
					// Since *rit is up to date, we can safely advance eit
					// to the end of *rit's range, and rit to the next
					eit = entries.lower_bound(rit->upper);
					++rit; // safe because of the sentinel
				}
			}
			return maxIndex;
		}

		void put(const Key_t &key, const Datum_t &datum)
		{
			using namespace std;
			// TODO: implement random tie breaking
			if (size == capacity) {
				// evict the group with largest count
				int maxIndex = computeGroups();
				typename PidSet::iterator it_end = groups[maxIndex].pids.end();
                int totalflushed = 0;
				for (typename PidSet::iterator it = groups[maxIndex].pids.begin();
						it != it_end; ++it) {
					EntrySet::iterator start = entries.lower_bound(it->lower);
					EntrySet::iterator stop  = entries.lower_bound(it->upper);
					totalflushed += tree->put(start, stop);
					entries.erase(start, stop);
				}
                size -= totalflushed;
                //cerr<<"flushed "<<totalflushed<<" records"<<endl;
				// After evicting the largest group, 
                // cache pids from the second largest group
                pids.clear();
                Group *largest = groups+maxIndex;
                int left = config->batchKeepPidCount;
                while (left > 0) {
                    largest->count = 0;
                    largest = max_element(groups, groups+nGroups);
                    int x = (int) largest->pids.size();
                    if (x == 0)
                        break;
                    int toinsert = left >= x ? x : left;
                    // group's pids are already sorted in the order of 
                    // their lower bounds
                    pids.insert(pids.end(), largest->pids.begin(), largest->pids.begin()+toinsert);
                    left -= toinsert;
                }
				pids.push_back(maxPid);
				clearGroups();
			}

			entries.insert(Entry(key, datum));
			++size;
			// look up the current pid set
			typename PidSet::iterator it = 
				upper_bound(pids.begin(), pids.end(), key, comp);
			if (it != pids.begin() && --it != pids.end() &&
					(it->lower <= key && it->upper > key)) {
				it->count++;
			}
		}

		void flushAll()
		{
			BatchBuffer::flushAll();
			pids.clear();
			pids.push_back(maxPid);  // serve as sentinel
			clearGroups();
		}

private:
		int nGroups;
		Group *groups;
		PidSet pids;
		static CompPid<PageId> comp;
		static PageId maxPid;

		void clearGroups()
		{
			for (int i=0; i<nGroups; ++i)
				groups[i].clear();
		}
	};

	template<class PageId>
	CompPid<PageId> BatchBufferLG<PageId>::comp;
	template<class PageId>
	PageId BatchBufferLG<PageId>::maxPid(MAX_KEY, MAX_KEY, 0);
}
