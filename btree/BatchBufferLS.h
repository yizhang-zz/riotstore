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

		typedef std::vector<PageId> PidList;

        // size of pageId = lower+upper+count=8+8+4
        // size of entry  = key+datum=8+8
		BatchBufferLS(u32 cap_, BTree *tree_): BatchBuffer(
				cap_-(8+8+4)*config->batchKeepPidCount/(8+8)
				, tree_)
		{
            using namespace std;
            cerr<<"Effective capacity = "<<capacity<<endl;
			nKeep = 1+config->batchKeepPidCount;
			pids = new PidList;
            pids->reserve(nKeep);
            pids->push_back(maxPid); // sentinel
            entries.insert(Entry(MAX_KEY, 0)); // sentinel
		}

        ~BatchBufferLS()
        {
            // remove the sentinel
            entries.erase(MAX_KEY);
            tree->put(entries.begin(), entries.end());
            delete pids;
        }

		PageId computePids()
		{
			using namespace std;
            PageId ret;
            unsigned knownCount = 0;
			EntrySet::iterator eit = entries.begin();
			typename PidList::iterator rit = pids->begin(),
                     lit=pids->end();
			while ((ret.count) < size - knownCount) {
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
                    if (npid.count > ret.count) {
                        ret = npid;
                        if (pids->size() < nKeep) {
                            lit = pids->insert(rit, npid);
                            ++(rit=lit);
                        }
                    }
                    else if (pids->size() < nKeep) {
                        rit = pids->insert(rit, npid);
                        ++rit;
                    }
				}
				else {
                    if (rit->count > ret.count) {
                        ret = *rit;
                        lit = rit;
                    }
                    knownCount += rit->count;
					// Since *rit is up to date, we can safely advance eit
					// to the end of *rit's range, and rit to the next
					eit = entries.lower_bound(rit->upper);
					++rit; // safe because of the sentinel
				}
			}

            if (lit != pids->end()) {
                pids->erase(lit);
                // strictly speaking we need to cache one more pid to make
                // sure the total number of pids is nKeep
            }
            return ret;
		}

        // don't cache pids
		PageId computePids1()
        {
            unsigned knownCount = 0;
			EntrySet::const_iterator eit = entries.begin();
            PageId ret;
            while (ret.count < size - knownCount) {
                assert(eit->key != MAX_KEY);
                PageId npid;
                tree->locate(eit->key, npid);
                // find all records going into npid
                while (eit->key < npid.upper) {
                    ++eit;
                    ++npid.count;
                }
                knownCount += npid.count;
                if (npid.count > ret.count) {
                    ret = npid;
                }
            }
            return ret;
        }

		void put(const Key_t &key, const Datum_t &datum)
		{
			using namespace std;
			if (size == capacity) {
				// evict the pid with largest count
				PageId longest = computePids();
				EntrySet::iterator start = entries.lower_bound(longest.lower);
				EntrySet::iterator stop  = entries.lower_bound(longest.upper);
                int c = tree->put(start, stop);
                //cerr<<c<<endl;
                assert(c == longest.count);
				entries.erase(start, stop);
				size -= longest.count;
			}

			entries.insert(Entry(key, datum));
			++size;
            //return;
			// Look up the current pid set for key's pid. Because of the
            // sentinel, it won't be pids->end().
			typename PidList::iterator it = 
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
		PidList *pids;
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
