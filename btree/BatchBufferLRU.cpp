#include "btree/BatchBufferLRU.h"
#include "btree/Btree.h"
#include <iostream>

using namespace Btree;
using namespace std;

BatchBufferLRU::~BatchBufferLRU()
{
	flushAll();
}

void BatchBufferLRU::put(const Key_t &key, const Datum_t &datum)
{
	if (size == capacity) {
		tree->put(head->entries.begin(), head->entries.end());
		size -= head->entries.size();
		pidMap.erase(head->pid);
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
		if (p->lower <= key && p->upper > key) {
			p->entries.insert(Entry(key, datum));
			break;
		}
		p = p->next;
	}
	if (p)
		goto put_done;
	
	// first of its kind
	p = new Node();
	tree->locate(key, p->pid, p->lower, p->upper);
	p->entries.insert(Entry(key, datum));
	pidMap[p->pid] = p;

 put_done:
	++size;
	// remove p from list
	if (p->prev)
		p->prev->next = p->next;
	else
		head = p->next;
	if (p->next)
		p->next->prev = p->prev;
	else
		tail = p->prev;
	
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

void BatchBufferLRU::flushAll()
{
	while (head) {
		tree->put(head->entries.begin(), head->entries.end());
		Node *p = head->next;
		delete head;
		head = p;
	}
	head = tail = NULL;
	pidMap.clear();
	size = 0;
}


bool BatchBufferLRU::find(const Key_t &key, Datum_t &datum)
{
	for (Node *p=head; p!=NULL; ++p) {
		for (Node::EntrySet::iterator it=p->entries.begin();
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

void BatchBufferLRU::print()
{
	Node *p = head;
	while (p) {
		cout<<"PID="<<p->pid<<" ["<<p->lower<<","<<p->upper<<"): ";
		Node::EntrySet::iterator it = p->entries.begin();
		for (; it != p->entries.end(); ++it)
			cout<<it->key<<" ";
		cout<<endl;
		p = p->next;
	}
}

