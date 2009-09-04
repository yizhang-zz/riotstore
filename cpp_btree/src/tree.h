#ifndef TREE_H
#define TREE_H

#define ROOT 1
#define INTERNAL 2
#define LEAF 4

#define INS_OVERFLOW 11
#define INS_NORMAL 12
#define INS_DENSE 13


class BTree {
   public:
      int leafCount;
      Node root;
      int order;
      int depth;
      Node firstLeaf;

      static int count = 0;

   private:
      int internalCount;

   public:
      BTree(int order, int start, int end);
      void insert(int key, double data);
      SearchResult search(int key);
};

class Node {
    private:
        int lowerBound, upperBound;
        int type;
        int size;
        static int count = 0;

    public:
        static Splitter splitter;
        static boolean useDense = false;
        static int denseCapacity;

    public:
        int capacity;
        int* keys;
        double* vals;
        Node* ptrs;

        Node parent;
        int index; // index in parent, quick access to neighbors
        BTree tree; // only for root
        Node pre; // only for leaves
        Node next;

        int id;
        bool inserted = false;
        bool dense;

    public:
        static int getBoundary();
        Node(int order, int type);

        void setRange(int start, int end);
        Range getRange();
        int getLowerBound();
        int getUpperBound();
        void setLowerBound(int l);
        void setUpperBound(int u);
        int getSize();
        
        bool isLeaf();
        bool isRoot();
        bool isInternal();
        
        void insert(Node n);
        int insert(int key, double val);
        int locate(int key);
        SearchResult search(int key);
};

#endif
