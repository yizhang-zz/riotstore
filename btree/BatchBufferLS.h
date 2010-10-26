#pragma once

#include "BatchBuffer.h"
#include <list>
#include <boost/pool/pool.hpp>

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
			
			void *operator new(size_t size)
			{
				return pool.malloc();
			}
			void operator delete(void *p)
			{
				pool.free(p);
			}

		private:
			static boost::pool<> pool;
		};

		typedef std::list<Node*> NodeList;
		typedef std::map<int, int> CountMap;

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
				int longestLen = list.front()->entries.size();
				CountMap::iterator cit = typeCount.find(longestLen);
				int count = cit->second;
				// sample one from count number of elements
				int selected = rand() % count;
				typename NodeList::iterator it = list.begin();
				for (int i=0; i<selected; ++i)
					++it;
				tree->put((*it)->entries.begin(), (*it)->entries.end());
				size -= longestLen;
				delete *it;
				list.erase(it);
				//--typeCount[longestLen];
				cit->second--;
			}

			bool found = false;
			typename NodeList::iterator it = list.begin();
			for (; it != list.end(); ++it) {
				if ((*it)->pid.contains(key)) {
					size_t nodeSize = (*it)->entries.size();
					CountMap::iterator cit = typeCount.find(nodeSize);
					--cit->second;
					(*it)->entries.insert(Entry(key, datum));
					typeCount[++nodeSize]++;

					// make sure list is still sorted
					typename NodeList::iterator t_it = it;
					Node *node = *it;
					do {
						--t_it;
					} while ( t_it != list.end()
							&& (*t_it)->entries.size() < nodeSize );
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
				Node *node = new Node;
				tree->locate(key, node->pid);
				node->entries.insert(Entry(key, datum));
				list.push_back(node);
				typeCount[1]++;
			}
			++size;
		}

		void flushAll()
		{
			typename NodeList::iterator it = list.begin();
			for (; it != list.end(); ++it) {
				tree->put((*it)->entries.begin(), (*it)->entries.end());
				delete *it;
			}
			list.clear();
			typeCount.clear();
		}

		bool find(const Key_t &key, Datum_t &datum)
		{
			typename NodeList::iterator it = list.begin();
			typename Node::EntrySet::iterator eit;
			Entry target(key, 0.0);
			for (; it != list.end(); ++it) {
				if ((eit=(*it)->entries.find(target)) != (*it)->entries.end()) {
					datum = eit->datum;
					return true;
				}
			}
			return false;
		}

		void print()
		{
			using namespace std;
			CountMap::iterator mit = typeCount.begin();
			cout<<"Type Count:: ";
			for (; mit != typeCount.end(); ++mit)
				cout<<mit->first<<":"<<mit->second<<" ";
			cout<<endl;
			typename NodeList::iterator it = list.begin();
			typename Node::EntrySet::iterator eit;
			for (; it != list.end(); ++it) {
				cout<<"PID="<<(*it)->pid<<" NUM="<<(*it)->entries.size()<<" ";
				typename Node::EntrySet::iterator eit = (*it)->entries.begin();
				for (; eit != (*it)->entries.end(); ++eit)
					cout<<eit->key<<", ";
				cout<<endl;
			}
		}


		NodeList list;
		CountMap typeCount;
	};

	template<class PageId>
	boost::pool<> BatchBufferLS<PageId>::Node::pool(sizeof(Node));
}
