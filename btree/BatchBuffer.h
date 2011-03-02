#ifndef BTREE_BATCH_BUFFER_H
#define BTREE_BATCH_BUFFER_H
#pragma once

#include "common/common.h"
#include <iostream>
#include <iterator>
#include <set>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/global_fun.hpp>

namespace Btree
{
class BTree;

enum BatchMethod {
	kNone,
	kFWF,
	kLRU,
	kLS,
    kSP,
	kLS_RAND,
	kLS_RANDCUT,
	kLG,
	kLG_RAND,
    kLPALL
};

class BoundPageId
{
public:
	BoundPageId(): lower(0), upper(0), count(0)
	{
	}

	BoundPageId(Key_t l, Key_t u, u16 c): lower(l), upper(u), count(c)
	{
	}

	bool contains(Key_t key)
	{
		return (lower<=key && upper>key);
	}

    bool operator< (const BoundPageId &other) const
    {
        return lower < other.lower;
    }

	friend std::ostream & operator<<(std::ostream &out, const BoundPageId &page);

	Key_t lower, upper;
	unsigned int count; // how many records to go into this page
};

inline std::ostream & operator<<(std::ostream &out, const Btree::BoundPageId &page)
{
	out<<"["<<page.lower<<","<<page.upper<<") <"<<page.count<<">";
	return out;
}

template<class Pid>
struct CompPid
{
	bool operator()(const Pid &p, Key_t k) const { return p.lower < k; }
	bool operator()(Key_t k, const Pid &p) const { return k < p.lower; }
	bool operator()(const Pid &p, const Pid &q) const { return p.lower < q.lower; }
};

template<class Pid>
struct CompPidCount
{
	bool operator()(const Pid &p, const Pid &q) const { return p.count < q.count; }
};

template<class Pid>
struct CompPidCountR
{
	bool operator()(const Pid &p, const Pid &q) const { return p.count > q.count; }
};

template<class Pid>
struct ChangePidCount
{
	ChangePidCount(u16 c):count(c) {}
	void operator()(Pid &pid)
	{
		pid.count = count;
	}
private:
	u16 count;
};

inline Key_t entryKey(Entry *p) { return p->key; }

class BatchBuffer
{
public:
	typedef boost::multi_index::multi_index_container<Entry,
			boost::multi_index::indexed_by<
				boost::multi_index::ordered_unique<
				boost::multi_index::member<Entry, Key_t, &Entry::key> > > > EntrySet;

    typedef boost::multi_index::multi_index_container<Entry*,
            boost::multi_index::indexed_by<
                boost::multi_index::ordered_unique<
                boost::multi_index::global_fun<Entry*, Key_t, &entryKey> > > > EntryPtrSet;

    class Iterator
    {
    public:
        Iterator(EntryPtrSet::iterator it_) : it(it_) { }
        bool operator!= (const Iterator &other) const { return it != other.it; }
        Iterator & operator++() { ++it; return *this; }
        Entry * operator->() { return *it; }
    private:
        EntryPtrSet::iterator it;
    };

    /*
	struct CompEntry
	{
		bool operator()(Key_t k, const Entry &e) const {return k<e.key;}
		bool operator()(const Entry &e, Key_t k) const {return e.key<k;}
	};

	const static CompEntry compEntry;
    */
	BatchBuffer(u32 cap_, BTree *tree_):capacity(cap_), size(0), tree(tree_)
	{
	}

	virtual ~BatchBuffer()
	{
	}

	virtual bool find(const Key_t &key, Datum_t &datum);

	virtual void flushAll();

	virtual void put(const Key_t &key, const Datum_t &datum) = 0;

	//virtual void print();

protected:
	u32 capacity;
	u32 size;
	BTree *tree;
	EntrySet entries;
};
}

#endif
