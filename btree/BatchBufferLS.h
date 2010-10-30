#pragma once

#include "BatchBuffer.h"
#include <iostream>
#include <iterator>
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

		typedef multi_index_container<PageId,
				indexed_by<
					ordered_unique<member<PageId, Key_t, &PageId::lower> >,
					ordered_non_unique<member<PageId, u16, &PageId::count>, std::greater<u16> >
				> > PidSet;
		typedef typename PidSet::template nth_index<0>::type PidSetByRange;
		typedef typename PidSet::template nth_index<1>::type PidSetByCount;

		BatchBufferLS(u32 cap_, BTree *tree_): BatchBuffer(
				cap_-sizeof(PageId)*config->batchKeepPidCount/sizeof(Entry)
				, tree_), knownCount(0)
		{
		}

		~BatchBufferLS()
		{
		}

		void computePids()
		{
			EntrySet::iterator eit = entries.begin();
			PidSetByRange &byRange = pids.get<0>();
			typename PidSetByRange::iterator rit = byRange.begin();
			Key_t lower;
			if (byRange.size() == 0)
				lower = MAX_KEY;
			else {
				//rit = byRange.begin();
				lower = rit->lower;
			}
			while (eit != entries.end()) {
				if (eit->key < lower) {
					if (!needsComputePids())
						break;
					PageId npid;
					tree->locate(eit->key, npid);
					// find the last record that goes into npid
					while (eit != entries.end() && eit->key < npid.upper) {
						++eit;
						++npid.count;
					}
					// insert the new pid
					byRange.insert(rit, npid);
					knownCount += npid.count;
					// rit hasn't moved
				}
				else {
					// Since *rit is up to date, we can safely advance eit
					// to the end of *rit's range, and rit to the next
					eit = entries.lower_bound(rit->upper, compEntry);
					++rit;
					if (rit != byRange.end())
						lower = rit->lower;
					else
						lower = MAX_KEY;
				}
			}
		}

		bool needsComputePids()
		{
			PidSetByCount &byCount = pids.get<1>();
			typename PidSetByCount::iterator it = byCount.begin();
			u16 x = it->count;
			return !(byCount.size() != 0 && x >= (++it)->count + (size-knownCount)); 
		}

		void put(const Key_t &key, const Datum_t &datum)
		{
			// TODO: implement random tie breaking
			if (size == capacity) {
				// try shortcut first: if first.count-second.count >= #records whose membership is unknown yet, 
				// then the first will be chosen no matter what. If random tie breaking is required, then the
				// above condition should be changed to >
				PidSetByCount &byCount = pids.get<1>();
				if (needsComputePids())
					computePids();
				typename PidSetByCount::iterator it = byCount.begin();

				// evict the pid with largest count
				EntrySet::iterator start = entries.lower_bound(it->lower, compEntry);
				EntrySet::iterator stop  = entries.lower_bound(it->upper, compEntry);
				tree->put(start, stop);
				entries.erase(start, stop);
				size -= it->count;
				knownCount -= it->count;
				byCount.erase(it);

				// keep only limited number of pids
				if (byCount.size() > config->batchKeepPidCount) {
					it = byCount.begin();
					knownCount = 0;
					for (u16 i=0; i<config->batchKeepPidCount; ++i) {
						knownCount += it->count;
						++it;
					}
					byCount.erase(it, byCount.end());
				}
			}

			entries.insert(Entry(key, datum));
			++size;
			// look up the current pid set
			PidSetByRange &byRange = pids.get<0>();
			typename PidSetByRange::iterator it = byRange.upper_bound(key);
			if (it != byRange.begin() && --it != byRange.end() &&
					(it->lower <= key && it->upper > key)) {
				u16 newcount = it->count + 1;
				byRange.modify(it, ChangePidCount<PageId>(newcount));
				++knownCount;
			}
		}

		void flushAll()
		{
			BatchBuffer::flushAll();
			knownCount = 0;
		}

		void print()
		{
			// first print all records and then the pid table
			using namespace std;
			BatchBuffer::print();
			PidSetByRange &byRange = pids.get<0>();
			ostream_iterator<PageId> output(cout, "\n");
			copy(byRange.begin(), byRange.end(), output);
			cout<<endl;
		}
private:
		PidSet pids;
		int knownCount;
	};

	/*
	template<class PageId>
	boost::pool<> BatchBufferLS<PageId>::Node::pool(sizeof(Node));
	*/
}
