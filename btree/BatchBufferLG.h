#ifdef USE_BATCH_BUFFER
#pragma once

#include "BatchBuffer.h"
#include <list>

namespace Btree
{
	template<class PageId>
	class BatchBufferLG: public BatchBuffer
	{
	public:
		struct Node
		{
			typedef std::set<Entry> EntrySet;
			PageId pid;
			EntrySet entries;
		};

		struct Group
		{
			typedef std::list<typename std::list<Node>::iterator> MembersType;
			u32 size;
			MembersType members;

			void clear()
			{
				size = 0;
				members.clear();
			}
		};

		BatchBufferLG(u32 cap_, BTree *tree_): BatchBuffer(cap_, tree_)
		{
			// There are ceiling(log(cap,2)) number of groups.
			// Group j contains pages with size [2^j,2^(j+1)), j=0,...
			numGroups = floorLog2(cap_-1)+1;
			groups = new Group[numGroups];
		}

		~BatchBufferLG()
		{
			delete[] groups;
		}

		void clearGroups()
		{
			for (int i=0; i<numGroups; ++i)
				groups[i].clear();
		}

		void put(const Key_t &key, const Datum_t &datum)
		{
			if (size == capacity) {
				clearGroups();
				int iMaxGroup;
				u32 sizeMaxGroup = 0;
				typename std::list<Node>::iterator it = list.begin();
				for (; it != list.end(); ++it) {
					size_t s = it->entries.size();
					int iGroup = floorLog2(s);
					groups[iGroup].size += s;
					groups[iGroup].members.push_back(it);
					if (groups[iGroup].size > sizeMaxGroup) {
						iMaxGroup = iGroup;
						sizeMaxGroup = groups[iGroup].size;
					}
				}
				typename Group::MembersType::iterator mit
				   	= groups[iMaxGroup].members.begin();
				for (; mit != groups[iMaxGroup].members.end(); ++mit) {
					tree->put((*mit)->entries.begin(), (*mit)->entries.end());
					size -= (*mit)->entries.size();
					list.erase(*mit);
				}
			}

			bool found = false;
			typename std::list<Node>::iterator it = list.begin();
			for (; it != list.end(); ++it) {
				if (it->pid.contains(key)) {
					it->entries.insert(Entry(key, datum));
					found = true;
					break;
				}
			}
			if (!found) {
				list.push_back(Node());
				tree->locate(key, list.back().pid);
				list.back().entries.insert(Entry(key, datum));
			}
			++size;
		}

		void flushAll()
		{
			typename std::list<Node>::iterator it = list.begin();
			for (; it != list.end(); ++it) {
				tree->put(it->entries.begin(), it->entries.end());
			}
			list.clear();
		}

		bool find(const Key_t &key, Datum_t &datum)
		{
			typename std::list<Node>::iterator it = list.begin();
			typename Node::EntrySet::iterator eit;
			Entry target(key, 0.0);
			for (; it != list.end(); ++it) {
				if ((eit=it->entries.find(target)) != it->entries.end()) {
					datum = eit->datum;
					return true;
				}
			}
			return false;
		}

		void print()
		{
			using namespace std;
			typename std::list<Node>::iterator it = list.begin();
			typename Node::EntrySet::iterator eit;
			for (; it != list.end(); ++it) {
				cout<<"PID="<<it->pid<<" ";
				typename Node::EntrySet::iterator eit = it->entries.begin();
				for (; eit != it->entries.end(); ++eit)
					cout<<eit->key<<", ";
				cout<<endl;
			}
		}


		std::list<Node> list;
		Group *groups;
		int numGroups;
	};
}
#endif
