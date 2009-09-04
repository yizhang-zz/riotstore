#include "tree.h"

int Node::getBoundary() {
   if (useDense)
      return (int)(denseCapacity * 1.5);
   return denseCapacity;
}

Node::Node(int order, int type) {
   dense = false;
   capacity = order;
   this.type = type;
   size = 0;
   keys = new int[((int)(capacity * 1.5)) + 1];
   id = count++;
   if (isInternal() || isRoot())
      ptrs = new Node[order];
   else if (isLeaf())
      vals = new double[((int)(capacity * 1.5)) + 1];
}

void Node::setRange(int start, int end) {
   setLowerRange(start);
   setLowerRange(end);
   if (!isLeaf()) {
      size = 1;
      keys[0] = start;
      keys[1] = end;
   }
}

Range Node::getRange() {
   return new Range(lowerBound, upperBound);
}

int Node::getLowerBound() {
   return lowerBound;
}


int Node::getUpperBound() {
   return upperBound;
}

void Node::setLowerBound(int l) {
   lowerBound = l;
}

void Node::setUpperBound(int u) {
   upperBound = u;
}

int Node::getSize() {
   return size;
}

bool Node::isLeaf() {
   return (type & LEAF) != 0;
}

bool Node::isInternal() {
   return (type & INTERNAL) != 0;
}

bool Nod::isRoot() {
   return (type & ROOT) != 0;
}        

/**
 * Insert a new child to an internal (or root) node.
 * 
 * @param n
 *            the new child
 */
public void insert(Node n) {
   if (isLeaf())
      return;
   int lb = n.getLowerBound();
   Node n1 = new Node(capacity, INTERNAL);
   boolean passed = false;
   if (size == capacity) {
      int sp = (size + 1) / 2; // size of first half
      if (lb > keys[sp - 1]) {
         int j = 0;
         // copy sp ... size-1 to a new sibling, and add n to it
         for (int i = sp; i < size; i++, j++) {
            if (keys[i] > lb && !passed) {
               n1.keys[j] = lb;
               n1.ptrs[j] = n;
               n.parent = n1;
               n.index = j;
               // j++;
               // n1.keys[j] = keys[i];
               // n1.ptrs[j] = ptrs[j];
               // n1.ptrs[j].parent = n1;
               passed = true;
               i--;
            }
            else {
               // if (keys[i+offset] < lb) {
               n1.keys[j] = keys[i];
               n1.ptrs[j] = ptrs[i];
               n1.ptrs[j].parent = n1;
               n1.ptrs[j].index = j;
            }
         }
         if (!passed) {
            n1.keys[j] = lb;
            n1.ptrs[j] = n;
            n.parent = n1;
            n.index = j;
         }
         n1.size = this.size + 1 - sp;
         n1.setLowerBound(n1.keys[0]);
         n1.setUpperBound(this.getUpperBound());
         this.size = sp;
         this.setUpperBound(n1.getLowerBound());
      }
      else { // n should go into the first half
         // copy second half to new node
         for (int i = sp - 1; i < size; i++) {
            n1.keys[i - sp + 1] = keys[i];
            n1.ptrs[i - sp + 1] = ptrs[i];
            n1.ptrs[i - sp + 1].parent = n1;
            n1.ptrs[i - sp + 1].index = i - sp + 1;
         }
         n1.size = size + 1 - sp;
         n1.setUpperBound(this.getUpperBound());
         n1.setLowerBound(n1.keys[0]);
         // insert n into first half
         this.size = sp - 1;
         this.setUpperBound(n1.getLowerBound());
         // enough space for the actual insertion
         this.insert(n);
      }

      if (isRoot()) { // root
         Node r = tree.root = new Node(capacity, ROOT);
         r.size = 2;
         r.keys[0] = this.getLowerBound();
         r.keys[1] = n1.getLowerBound();
         r.keys[2] = n1.getUpperBound();
         r.ptrs[0] = this;
         this.index = 0;
         r.ptrs[1] = n1;
         n1.index = 1;
         this.type = INTERNAL;
         this.parent = n1.parent = r;
         r.tree = tree;
         r.tree.depth++;
         r.setLowerBound(this.getLowerBound());
         r.setUpperBound(n1.getUpperBound());
      } 
      else { // internal nodes
         parent.insert(n1);
      }
   }

   else {

      // space available
      int pos = locate(lb) + 1;
      keys[size + 1] = keys[size];
      for (int i = size - 1; i >= pos; i--) {
         keys[i + 1] = keys[i];
         ptrs[i + 1] = ptrs[i];
         ptrs[i + 1].index = i + 1;
      }
      keys[pos] = lb;
      ptrs[pos] = n;
      n.parent = this;
      n.index = pos;
      size++;
   }
}

/**
 * Insert a new (key,value) pair into a leaf node.
 * 
 * @param key
 *            the new key
 * @param val
 *            the new value
 * @return INS_OVERFLOW if a split occurs; INS_NORMAL otherwise.
 */
public int insert(int key, double val) {
   boolean split = false;
   if (!useDense)
      split = (size == capacity);
   else {
      if (!dense && size == capacity) {
         int maxSize = (int) (capacity * 1.5);
         // If actual range [first element, last element) can fit in
         // maxRange
         // then convert to dense format
         int first = Math.min(keys[0], key);
         int last = Math.max(keys[size - 1], key);
         // can be transformed into dense format
         if (last - first + 1 <= maxSize) {
            capacity = maxSize;
            dense = true;
            split = false;
            System.out.println("*** to dense");
         }
         // cannot, split
         else {
            split = true;
         }
      } 
      else if (dense && size == capacity) {
         //System.out.println("split dense");
         split = true;
      }
      else if (dense && size != capacity) {
         split = ((key < keys[0] && keys[size - 1] - key + 1 > capacity) || (key > keys[size - 1] && key
                  - keys[0] + 1 > capacity));
      }
   }
   if (split) {
      // find the position of the key
      int pos = locate(key) + 1;
      Node n1 = splitter.split(this, pos, key, val);
      parent.insert(n1);

      // link leaves together
      n1.next = this.next;
      this.next = n1;
      n1.pre = this;

      return INS_OVERFLOW;
   }

   justInsert = true;
   int pos = locate(key);
   // invariant: keys[pos]<=key<keys[pos+1]
   pos = pos + 1;
   for (int i = size - 1; i >= pos; i--) {
      keys[i + 1] = keys[i];
      vals[i + 1] = vals[i];
   }
   keys[pos] = key;
   vals[pos] = val;
   size++;
   return INS_NORMAL;
}

/**
 * If a key exists, return the index of that key in keys[]. Otherwise,
 * return the index of the key if the key is to be inserted. FIX: return i
 * such that keys[i]<= key < keys[i+1].
 * 
 * @param key
 * @return
 */
public int locate(int key) {
   if (size == 0)
      return -1;
   int p = 0, q = size - 1;
   int mid;
   do {
      mid = (p + q) / 2;
      if (keys[mid] > key)
         q = mid - 1;
      else if (keys[mid] < key)
         p = mid + 1;
      else
         return mid;
   } while (p <= q);
   return q;
}

public SearchResult search(int key) {
   if (type == LEAF) {
      int pos = locate(key);
      if (pos >= 0 && keys[pos] == key)
         return new SearchResult(SearchResult.Status.FOUND, this, key,
               vals[pos]);
      else
         return new SearchResult(SearchResult.Status.NONE, this, key, 0);
   }
   else {
      int pos = locate(key);
      return ptrs[pos].search(key);
   }
}

