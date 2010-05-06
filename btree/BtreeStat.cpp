#include "BtreeStat.h"

using namespace Btree;

BtreeStat::BtreeStat(int range, int nBins)
{
  interval = range / nBins;
  if (nBins * interval < range)
	interval++;
  for (int i=0; i<nBins; i++) {
	histogram[i*interval] = 0;
  }
}

double BtreeStat::getTotal()
{
  double r = 0.0;
  std::map<int,int>::const_iterator it = histogram.begin();
  // approximate sum of a bin by #elements * middle value of bin
  for (; it != histogram.end(); ++it)
	r += it->second * (it->first + interval/2);
  return r;
}
