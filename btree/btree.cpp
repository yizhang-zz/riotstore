#include "tree.h"


BTree::BTree(int order, int start, int end) {
    this.order = order;
    root = new Node(order, Node.ROOT);
    root.setRange(start, end);
    root.tree = this;
    firstLeaf = new Node(order, Node.LEAF);
    firstLeaf.setRange(start, end);
    // first contains one entry with key=start
    root.ptrs[0] = firstLeaf;
    firstLeaf.index = 0;
    firstLeaf.parent = root;
    depth = 1;
    leafCount = 1;
}

void BTree::insert(int key, double data) {
    SearchResult sr = search(key);
    int re = sr.node.insert(key, data);
    if (re == Node.INS_OVERFLOW)
        leafCount++;
    cout << leafCount << endl;
}
    
SearchResult BTree::search(int key) {
    return root.search(key);
}
