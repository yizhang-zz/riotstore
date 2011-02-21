#ifndef BATCHBUFFER_LSRAND_H
#define BATCHBUFFER_LSRAND_H

#include "BatchBuffer.h"
#include <vector>

namespace Btree
{
	template<class PageId>
	class BatchBufferLSRand: public BatchBuffer
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

        void add(Entry e)
        {
            assert(freehead != -1);
            int next = *(int*)(entriesArray+freehead);
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

        BatchBufferLSRand(u32 cap_, BTree *tree_):
            BatchBuffer(cap_/(1.0+ceilingLog2(cap_)/8/sizeof(Entry)), tree_)
        {
            using namespace std;
            gsl_rng_env_setup();
            if (gsl_rng_default_seed == 0) {
                gsl_rng_default_seed = time(NULL);
            }
            rng = gsl_rng_alloc(gsl_rng_taus2);
            entriesArray = new Entry[capacity];
            initFreeList();
        }

        ~BatchBufferLSRand()
        {
            tree->put(Iterator(entries.begin()), Iterator(entries.end()));
            delete[] entriesArray;
            gsl_rng_free(rng);
        }

        void print()
        {
            using namespace std;
            for(EntryPtrSet::iterator it=entries.begin();
                    it != entries.end(); ++it) {
                cerr<<(*it)->key<<" ";
            }
            cerr<<endl;
        }

        void put(const Key_t &key, const Datum_t &datum)
        {
            using namespace std;
            if (size == capacity) {
                int index = gsl_rng_uniform_int(rng, size);
                PageId pid;
                tree->locate(entriesArray[index].key, pid);
                EntryPtrSet::iterator begin = entries.lower_bound(pid.lower);
                EntryPtrSet::iterator end = entries.lower_bound(pid.upper);
                int ret = tree->put(Iterator(begin), Iterator(end));
                size -= ret;
                remove(begin, end);
            }

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
#endif
