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
				// find longest sequence/node
				typename std::list<Node>::iterator it = list.begin();
				int max = 0;
				typename std::list<Node>::iterator maxit;
				for (; it != list.end(); ++it) {
					if (it->entries.size() > max) {
						max = it->entries.size();
						maxit = it;
					}
				}
				tree->put(maxit->entries.begin(), maxit->entries.end());
				size -= max;
				list.erase(maxit);
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
			for (; it != list.end(); ++it) {
				if ((eit=it->entries.find(key)) != it->entries.end()) {
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
