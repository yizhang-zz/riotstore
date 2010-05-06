#ifndef BTREE_STAT_H
#define BTREE_STAT_H

#include <map>

namespace Btree
{
  class BtreeStat
  {
  public:
	/**
	 * Constructs a statistics object for a btree. Currently a
	 * histogram of the remaining capacity of leaf nodes is
	 * maintained.
	 *
	 * @param range The range of leaves' remaining capacity
	 * @param nBins The number of bins in the histogram
	 */
	BtreeStat(int range, int nBins);
	
	void update(int old, int current);
	double getTotal();
	std::map<int, int> histogram;
	int interval;
  };
}

#endif
