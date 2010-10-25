#pragma once

#include "BatchBuffer.h"
#include <list>

namespace Btree
{
	template<class PageId>
	class BatchBufferLS: public BatchBuffer
	{
	public:
		struct Node
		{
			typedef std::set<Entry> EntrySet;
			PageId pid;
			EntrySet entries;
		};

		BatchBufferLS(u32 cap_, BTree *tree_): BatchBuffer(cap_, tree_)
		{
		}

		~BatchBufferLS()
		{
		}

		void put(const Key_t &key, const Datum_t &datum)
		{
			if (size == capacity) {
				// list is already sorted according to length
				// break ties at random
				int longestLen = list.front().entries.size();
				int count = typeCount[longestLen];
				// sample one from count number of elements
				int selected = rand() % count;
				typename std::list<Node>::iterator it = list.begin();
				for (int i=0; i<selected; ++i)
					++it;
				tree->put(it->entries.begin(), it->entries.end());
				size -= longestLen;
				list.erase(it);
				--typeCount[longestLen];
			}

			bool found = false;
			typename std::list<Node>::iterator it = list.begin();
			for (; it != list.end(); ++it) {
				if (it->pid.contains(key)) {
					--typeCount[it->entries.size()];
					it->entries.insert(Entry(key, datum));
					size_t newSize = it->entries.size();
					++typeCount[newSize];

					// make sure list is still sorted
					typename std::list<Node>::iterator t_it = it;
					Node node = *it;
					while (t_it->entries.size() < newSize 
							&& t_it != list.end())
						--t_it;
					if (t_it == list.end()) {
						list.erase(it);
						list.push_front(node);
					}
					else {
						++t_it; // insert before this position
						if (t_it != it) {
							list.erase(it);
							list.insert(t_it, node);
						}
					}

					found = true;
					break;
				}
			}
			if (!found) {
				// length 1 node should always go to the tail of the list
				list.push_back(Node());
				tree->locate(key, list.back().pid);
				list.back().entries.insert(Entry(key, datum));
				++typeCount[1];
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
			typeCount.clear();
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
			std::map<int, int>::iterator mit = typeCount.begin();
			cout<<"Type Count:: ";
			for (; mit != typeCount.end(); ++mit)
				cout<<mit->first<<":"<<mit->second<<" ";
			cout<<endl;
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
		std::map<int, int> typeCount;
	};
}
