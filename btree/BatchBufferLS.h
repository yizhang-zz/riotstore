#ifndef BATCHBUFFERLS_H
#define BATCHBUFFERLS_H

#include "common/list.h"
#include "BatchBuffer.h"
#include "heap.hpp"
#include <iostream>
#include <iterator>
#include <boost/pool/pool.hpp>


using namespace boost;
using namespace boost::multi_index;

  struct Page
  {
    list_head list;
    Btree::BoundPageId page;
    int heap_index;
    void init()
    {
      page.count = 0;
      heap_index = 0;
      INIT_LIST_HEAD(&list);
    }
    Key_t key() const
    {
      return page.lower;
    }
  };
  struct Element
  {
    Entry entry;
    list_head node;
  };

  struct ElementIterator
  {
    ElementIterator(list_head *p)
    {
      ptr = p;
    }
    
    ElementIterator &operator++()
    {
      ptr = ptr->next;
      return *this;
    }

    bool operator!=(const ElementIterator &other)
    {
      return ptr != other.ptr;
    }

	Entry *operator->()
	{
		return &(list_entry(ptr, Element, node)->entry);
	}

  private:
    list_head *ptr;
  };
namespace Btree
{
	inline Key_t pageKey(Page* p)
	{
		return p->page.lower;
	}
template<class PageId>
class BatchBufferLS: public BatchBuffer
{
public:

  typedef boost::multi_index::multi_index_container<
    Page*,
    boost::multi_index::indexed_by<
      boost::multi_index::ordered_unique<
	boost::multi_index::global_fun<Page*, Key_t, &pageKey> 
	> > > PidSet;

  PidSet pids;
  int nPids;
  boost::pool<> ppool; // for pages
  boost::pool<> epool; // for elements
  Heap heap;

  BatchBufferLS(u32 cap_, BTree *tree_): BatchBuffer(cap_, tree_),
					 nPids(0),
					 ppool(sizeof(Page)),
					 epool(sizeof(Element)),
					 heap(4000)
  {
  }

  ~BatchBufferLS()
  {
    flushAll();
    assert(nPids == 0);
  }

  int evictPage(Page *p)
  {
    ElementIterator start(p->list.next);
    ElementIterator stop(&p->list);
    int c = tree->put(start, stop);
    assert(c == p->page.count);
    size -= c;
    
    Element *e;
    list_head *end = &p->list;
    list_head *pos = p->list.next;
    list_head *tmp;
    for (; pos != end;)
      {
	e = list_entry(pos, Element, node);
	pos = pos->next;
	epool.free(e);
      }
    pids.erase(p->page.lower);
    ppool.free(p);
    nPids--;
	return c;
  }

  int evictLargest()
  {
    Page *p = heap.extractMax();
    return evictPage(p);
  }

  struct PagePtrComp
  {
    bool operator()(Key_t x, const Page *p) const
    {
      return x < p->page.lower;
    }

    bool operator()(const Page *p, Key_t x) const
    {
      return p->page.lower < x;
    }
  };

  bool isFull()
  {
    /* se=size of entry= 8+8 = 16.
       sp=size of pid= lower+upper+count+ptr_to_head_entry_list+heap_index.
       =8+8+4+2+4=26.
       sn=per node space in boost::multi_index_container (rbtree) = 24.
       total space of ALL: (se+sn)*capacity.

       total space of LS's entries: (se+2)*size, where 2=ptr for linked list.
       total space of LS's pids: y*(sp+4+4+24)
       =y*(sp+ptr_in_rbtree+ptr_in_heap+sn).
    */
    return (16+2)*size + (26+4+4+24)*nPids >= (16+24)*capacity;
  }
    
  void put(const Key_t &key, const Datum_t &datum)
  {
    using namespace std;

    if (isFull()) {
	int n = evictLargest();
		//cerr<<size<<"\t"<<nPids<<"\t"<<n<<endl;
	}

    PidSet::iterator it = pids.upper_bound(key/*, PagePtrComp()*/);
    if (pids.size() == 0 || it == pids.begin() ||!(*(--it))->page.contains(key))
      createPage(Entry(key, datum));
    else
      addToPage(Entry(key, datum), *it);
  }

  void createPage(const Entry &e)
  {
    Page *p = (Page *)ppool.malloc();
    p->init();
    tree->locate(e.key, p->page);
    heap.insert(p);
	pids.insert(p);
    addToPage(e, p);
    nPids++;
  }

  void addToPage(const Entry &e, Page *p)
  {
    Element *elem = (Element*) epool.malloc();
    elem->entry = e;
    list_add(&elem->node, &p->list);
    p->page.count++;
    heap.onChange(p->heap_index);
    size++;
  }

  void flushAll()
  {
    PidSet::iterator it = pids.begin();
    for (; it != pids.end(); ++it)
      evictPage(*it);
    pids.clear();
    assert(nPids == 0);
  }

  bool find(const Key_t &key, Datum_t &datum)
  {
    PidSet::iterator it = pids.upper_bound(key);
    if (pids.size() == 0 || it == pids.begin() ||!(*(--it))->page.contains(key))
      return false;
    list_head *head = &(*it)->list;
    list_head *pos;
    Element *elem;
    Key_t k = key;
    list_for_each(pos, head)
      {
	elem = list_entry(pos, Element, node);
	if (elem->entry.key == k)
	  {
	    datum = elem->entry.datum;
	    return true;
	  }
      }
    return false;
  }
};

}
#endif
