#pragma once

#ifdef USE_BATCH_BUFFER

#include "btree/BatchBuffer.h"
#include <map>
#include <set>
#include <iterator>

namespace Btree
{
	class BoundPageId
	{
	public:
		BoundPageId(): lower(0), upper(0)
		{
		}

		BoundPageId(Key_t l, Key_t u): lower(l), upper(u)
		{
		}

		bool contains(Key_t key)
		{
			return (lower<=key && upper>key);
		}

		friend std::ostream & operator<<(std::ostream &out, BoundPageId &page);
		Key_t lower, upper;
	};

	inline std::ostream & operator<<(std::ostream &out, BoundPageId &page)
	{
		out<<"["<<page.lower<<","<<page.upper<<")";
		return out;
	}

	template<class PageId>
class BatchBufferLRU : public BatchBuffer
{
private:
	struct Node
	{
		typedef std::set<Entry> EntrySet;
		PageId pid;
		//Key_t lower, upper;
		std::set<Entry> entries;
		Node *prev;
		Node *next;

		Node() : prev(0), next(0)
		{
		}
		
		//Node(PageId p, Key_t l, Key_t u)
		//	:pid(p),lower(l),upper(u),prev(0),next(0)
		Node(PageId p) : pid(p)
		{
		}
	};
public:
	BatchBufferLRU(u32 cap_, BTree *tree_):BatchBuffer(cap_,tree_),
										   head(0), tail(0)
	{
	}

	virtual ~BatchBufferLRU()
	{
		flushAll();
	}

	virtual void put(const Key_t &key, const Datum_t &datum)
	{
		if (size == capacity) {
			tree->put(head->entries.begin(), head->entries.end());
			size -= head->entries.size();
			//pidMap.erase(head->pid);
			Node *temp = head->next;
			delete head;
			head = temp;
			if (head)
				head->prev = NULL;
			else
				tail = NULL;
		}

		Node *p = head;
		while (p != NULL) {
			//if (p->lower <= key && p->upper > key) {
			if (p->pid.contains(key)) {
				p->entries.insert(Entry(key, datum));
				break;
			}
			p = p->next;
		}
		if (p) {
			// remove p from list
			if (p->prev)
				p->prev->next = p->next;
			else
				head = p->next;
			if (p->next)
				p->next->prev = p->prev;
			else
				tail = p->prev;
		}
		else {
			// creat new node because it's first of its kind
			p = new Node();
			tree->locate(key, p->pid);
			p->entries.insert(Entry(key, datum));
			//pidMap[p->pid] = p;
		}

		++size;

		// place p to tail
		if (head == NULL) {
			head = tail = p;
			p->prev = p->next = NULL;
		}
		else {
			p->prev = tail;
			p->next = NULL;
			tail->next = p;
			tail = p;
		}
	}
	
	virtual void flushAll()
	{
		while (head) {
			tree->put(head->entries.begin(), head->entries.end());
			Node *p = head->next;
			delete head;
			head = p;
		}
		head = tail = NULL;
		//pidMap.clear();
		size = 0;
	}
	
	virtual bool find(const Key_t &key, Datum_t &datum)
	{
		for (Node *p=head; p!=NULL; p=p->next) {
			for (typename Node::EntrySet::iterator it=p->entries.begin();
					it != p->entries.end();
					++it) {
				if (it->key == key) {
					datum = it->datum;
					return true;
				}
			}
		}
		return false;
	}

	void print()
	{
		for (Node *p=head; p; p=p->next) {
			std::cout<<"PageId="<<p->pid;
			typename Node::EntrySet::iterator it = p->entries.begin();
			for (; it != p->entries.end(); ++it)
				std::cout<<it->key<<" ";
			std::cout<<std::endl;
		}
	}

	
	//std::map<PID_t, Node*> pidMap;
	Node *head;
	Node *tail;
};
}
#endif
