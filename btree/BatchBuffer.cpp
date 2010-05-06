#ifdef USE_BATCH_BUFFER
#include <math.h>
#include "BatchBuffer.h"
#include "BtreeStat.h"
#include <algorithm>

using namespace Btree;

BatchBuffer::BatchBuffer(u32 size)
{
  this->size = size;
}

void BatchBuffer::registerBTree(BTree *tree)
{
  assert(buffers.size()<2);
  buffers[tree] = NULL;
  tree->batbuf = this;
  assignBuffers();
}

void BatchBuffer::insert(Key_t key, Datum_t datum, BTree *where)
{
  std::vector<Entry> *v = buffers[where];
  if (capacities[where] == v->size()) {
	// buffer full
	flushBuffer(buffers.find(where));
  }

  Entry e;
  e.key = key;
  e.value.datum = datum;
  v->push_back(e);
}

void BatchBuffer::assignBuffers()
{
  // flush buffers
  for (Buffers::iterator it = buffers.begin(); it != buffers.end(); ++it)
	flushBuffer(it);
  u32 x = (u32) newton(); // how much buffer should the first B-tree have
  assert(x>=0 && x<=size);

  std::map<BTree*, std::vector<Entry>*>::iterator it = buffers.begin();
  capacities[it->first] = x;
  capacities[(++it)->first] = size-x;
}

bool compare_entry(const Entry &a, const Entry &b)
{
  return a.value.datum < b.value.datum;
}

void BatchBuffer::flushBuffer(Buffers::iterator it)
{
  if (it->second != NULL) {
	// sort to make insertions sequential
	std::vector<Entry> *v = it->second;
	std::sort(v->begin(), v->end(), compare_entry);
	BTree *tree = it->first;
	for (std::vector<Entry>::const_iterator j = v->begin();
		 j != v->end(); ++j) {
	  tree->put((*j).key, (*j).value.datum);
	}
	// clear buffers
	v->clear();
  }
}

double BatchBuffer::newton()
{
  // initial value
  double x = size/2;
  double y;
  double threshold = 1e-2;
  BtreeStat *stat[2];
  int i=0;
  std::map<BTree*, std::vector<Entry>*>::iterator it = buffers.begin();
  for (; it != buffers.end(); ++it) {
	stat[i++] = it->first->stat;
  }
  
  do {
	y = x;
	x = y - calc_g(y,stat[0],stat[1]) / calc_dg(y,stat[0],stat[1]);
  } while(fabs(x-y)>threshold);
  return x;
}

double BatchBuffer::calc_g(double x, BtreeStat *stat1, BtreeStat *stat2)
{
  return calc_f(x, stat1) - calc_f(size-x, stat2);
}

double BatchBuffer::calc_f(double x, BtreeStat *stat)
{
  double c = stat->getTotal();
  double r = 0.0;
  std::map<int,int>::iterator it = stat->histogram.begin();
  for(; it != stat->histogram.end(); ++it) {
	r += it->second * pow((c-it->first)/c, x) * (c-x+x*it->first)/(c-x);
	r -= it->second;
  }
  r /= x*x;
  return r;
}

double BatchBuffer::calc_dg(double x, BtreeStat *stat1, BtreeStat *stat2)
{
  return calc_df(x, stat1) + calc_df(size-x, stat2);
}

double BatchBuffer::calc_df(double x, BtreeStat *stat)
{
  double c = stat->getTotal();
  double r = -2/x*calc_f(x, stat);
  double s = 0.0;

  std::map<int,int>::iterator it = stat->histogram.begin();
  for(; it != stat->histogram.end(); ++it) {
	s += it->second * pow((c-it->first)/c, x) * 
	  ((c-x+x*it->first)/(c-x)*log((c-it->first)/c) + (c-it->first)/(c-x)/(c-x));
  }
  s /= x*x;
  return r+s;
}
#endif
