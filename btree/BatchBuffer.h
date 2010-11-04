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

namespace Btree
{
class BTree;

enum BatchMethod {
	kNone,
	kFWF,
	kLRU,
	kLS,
	kLS_RAND,
	kLS_RANDCUT,
	kLG,
	kLG_RAND
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

	friend std::ostream & operator<<(std::ostream &out, const BoundPageId &page);

	Key_t lower, upper;
	u16 count; // how many records to go into this page
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

class BatchBuffer
{
public:
	typedef boost::multi_index::multi_index_container<Entry,
			boost::multi_index::indexed_by<
				boost::multi_index::ordered_unique<
				boost::multi_index::identity<Entry> > > > EntrySet;

	struct CompEntry
	{
		bool operator()(Key_t k, const Entry &e) const {return k<e.key;}
		bool operator()(const Entry &e, Key_t k) const {return e.key<k;}
	};

	const static CompEntry compEntry;

	BatchBuffer(u32 cap_, BTree *tree_):capacity(cap_), size(0), tree(tree_)
	{
	}

	virtual ~BatchBuffer()
	{
		flushAll();
	}

	bool find(const Key_t &key, Datum_t &datum);

	virtual void flushAll();

	virtual void put(const Key_t &key, const Datum_t &datum) = 0;

	virtual void print();

protected:
	u32 capacity;
	u32 size;
	BTree *tree;
	EntrySet entries;
};
}

#endif
