#ifndef BTREE_H
#define BTREE_H

#define ROOT 1
#define INTERNAL 2
#define LEAF 4

#define INS_OVERFLOW 11
#define INS_NORMAL 12
#define INS_DENSE 13


class BTree {
   public:
      int leaf_count;
      pgno_t root;
	  DataBlock *root;
      int depth;
      pgno_t first_leaf;


   public:
      BTree(int capacity, key_t start, key_t end);
      int put(key_t key, datum_t data);
	  int put(key_t[] key, datum_t[] data, int size);
      SearchResult search(key_t key);
	  int get(key_t key, datum_t *datum);
	  int get(key_t begin, key_t end, datum_t *data);
	  int get(key_t *keys, datum_t *data, int size);
};

#endif
