#pragma once

#include <iostream>
#include "common/Config.h"
#include "BatchBufferLS.h"

namespace Btree
{
	template<class PageId>
	class BatchBufferLPALL: public BatchBufferLS<PageId>
    {
    public:

        typedef BatchBufferLS<PageId> parent;

        BatchBufferLPALL(u32 cap_, BTree *tree_): parent(cap_, tree_)
        {
        }

        void put(const Key_t &key, const Datum_t &datum)
        {
            using namespace std;
            if (this->size == this->capacity) {
                // try evict the pid with largest count
                PageId longest = this->computePids();

                // If largest page is small, just flush all
                if (config->flushPageMinSize > longest.count ) {
                    this->flushAll();
                    cerr<<"LP="<<longest.count<<" flush all..."<<endl;
                }
                else {
                    typename parent::EntrySet::iterator start = this->entries.lower_bound(longest.lower);
                    typename parent::EntrySet::iterator stop  = this->entries.lower_bound(longest.upper);
                    this->tree->put(start, stop);
                    this->entries.erase(start, stop);
                    this->size -= longest.count;
                }
            }

            this->entries.insert(Entry(key, datum));
            ++this->size;
            // look up the current pid set
            typename parent::PidSet::iterator it = 
                upper_bound(this->pids->begin(), this->pids->end(), key, this->comp);
            if (it != this->pids->begin() && --it != this->pids->end() &&
                    (it->lower <= key && it->upper > key)) {
                it->count++;
            }
        }
    };
}
