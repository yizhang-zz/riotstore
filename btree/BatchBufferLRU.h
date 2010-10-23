#pragma once

#include "btree/BatchBuffer.h"
#include <map>
#include <set>

namespace Btree
{
class BatchBufferLRU : public BatchBuffer
{
public:
	BatchBufferLRU(u32 cap_, BTree *tree_):BatchBuffer(cap_,tree_),
										   head(0), tail(0)
	{
	}

	virtual ~BatchBufferLRU();

	virtual void put(const Key_t &key, const Datum_t &datum);
	virtual void flushAll();
	virtual bool find(const Key_t &key, Datum_t &datum);
protected:
	struct Node
	{
		typedef std::set<Entry> EntrySet;
		PID_t pid;
		Key_t lower, upper;
		std::set<Entry> entries;
		Node *prev;
		Node *next;

		Node() : prev(0), next(0)
		{
		}
		
		Node(PID_t p, Key_t l, Key_t u)
			:pid(p),lower(l),upper(u),prev(0),next(0)
		{
		}
	};
	
	std::map<PID_t, Node*> pidMap;
	Node *head;
	Node *tail;
};
}
