#pragma once

#include "BatchBuffer.h"
#include <list>

namespace Btree
{
	template<class PageId>
	class BatchBufferLGRand: public BatchBuffer
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

		BatchBufferLGRand(u32 cap_, BTree *tree_): BatchBuffer(cap_, tree_)
		{
		}

		~BatchBufferLGRand()
		{
		}

		void put(const Key_t &key, const Datum_t &datum)
		{
			static Group g;
			if (size == capacity) {
				int index = rand() % size;
				typename std::list<Node>::iterator it = list.begin();
				while (it != list.end()) {
					index -= it->entries.size();
					if (index < 0)
						break;
					++it;
				}
				// the group *it belongs to will be flushed
				g.clear();
				int iGroup = floorLog2(it->entries.size());
				for (it = list.begin(); it != list.end(); ++it) {
					unsigned int s = it->entries.size();
					if (iGroup == floorLog2(s)) 
						g.members.push_back(it);
				}
				typename Group::MembersType::iterator mit
				   	= g.members.begin();
				for (; mit != g.members.end(); ++mit) {
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
	};
}
