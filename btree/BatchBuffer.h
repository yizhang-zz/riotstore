#ifndef BATCH_BUFFER_H
#define BATCH_BUFFER_H

#ifdef USE_BATCH_BUFFER

#include "../common/common.h"
#include "BtreeBlock.h"
#include "Btree.h"
#include "BtreeStat.h"
#include <set>
#include <vector>

namespace Btree
{
  class BatchBuffer
  {
  public:
	BatchBuffer(u32 size);
	~BatchBuffer();
	void registerBTree(BTree *tree);
	void insert(const Key_t &key, const Datum_t &datum, BTree *where);
	
	typedef std::map<BTree*, std::vector<Entry>*> Buffers;
  private:
	u32 size;
	Buffers buffers;
	std::map<BTree*, u32> capacities;

	void assignBuffers();
	void flushBuffer(Buffers::iterator it);

	// Newton-Ralphson method for finding the zero of g(x)
	double newton();
	// g(x) and g'(x)
	double calc_g(double x, BtreeStat *stat1, BtreeStat *stat2);
	double calc_dg(double x, BtreeStat *stat1, BtreeStat *stat2);
	// component of g(x) and its derivative
	double calc_f(double x, BtreeStat *stat);
	double calc_df(double x, BtreeStat *stat);
  };
}

#endif
#endif
