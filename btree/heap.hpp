#ifndef HEAP_HPP
#define HEAP_HPP

struct Page;

class Heap
{
public:
  Heap(int n);
  ~Heap();
  void onChange(int i);
  Page *extractMax();
  void insert(Page *p);
private:
  Page **buf;
  int buflen;
  int size;

  void maxHeapify(int i);
};

#endif
