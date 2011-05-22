#include "heap.hpp"
#include "Btree.h"
#include "BatchBufferLS.h"

#define KEY(x) ((x)->page.count)
#define UPDATE_INDEX(i) do{buf[i]->heap_index = i;} while (0)

/* Array index is 1-based; root is at buf[1]. */

static inline int parent(int i)
{
  return i>>1;
}

static inline int left(int i)
{
  return i<<1;
}

static inline int right(int i)
{
  return (i<<1)+1;
}

Heap::Heap(int n)
{
  buflen = n+1;
  size = 0;
  buf = (Page**) malloc(sizeof(Page*) * buflen);
}

Heap::~Heap()
{
  free(buf);
}

Page *Heap::extractMax()
{
  if (size < 1)
    return NULL;
  Page *max = buf[1];
  buf[1] = buf[size];
  UPDATE_INDEX(1);
  size--;
  maxHeapify(1);
  return max;
}

void Heap::maxHeapify(int i)
{
  int l = left(i);
  int r = right(i);
  int largest;
  if (l <= size && KEY(buf[l]) > KEY(buf[i]))
    largest = l;
  else
    largest = i;
  if (r <= size && KEY(buf[r]) > KEY(buf[largest]))
    largest = r;
  if (largest != i)
    {
      Page *temp = buf[i];
      buf[i] = buf[largest];
      buf[largest] = temp;
      UPDATE_INDEX(i);
      UPDATE_INDEX(largest);
      maxHeapify(largest);
    }
}

void Heap::onChange(int i)
{
  while (i > 1 && KEY(buf[parent(i)]) < KEY(buf[i]))
    {
      int j = parent(i);
      Page *temp = buf[i];
      buf[i] = buf[j];
      UPDATE_INDEX(i);
      buf[j] = temp;
      i = j;
    }
  UPDATE_INDEX(i);
}

void Heap::insert(Page *p)
{
  if (buflen == size + 1)
    {
      buf = (Page**) realloc(buf, sizeof(Page*) * (size*2+1));
      buflen = size*2+1;
    }
  size++;
  buf[size] = p;
  // all keys are nonnegative, so the smallest possible is 0
  //if (KEY(p) != 0)
    onChange(size);
}
