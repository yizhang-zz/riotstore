#ifndef UTILS_H
#define UTILS_H

enum Status { NONE, FOUND};
class Node;

class Range {
   public:
      int start;
      int end;

   public:
      Range(int s, int e);
};

class SearchResult {
    public:
       Status status;
       Node node;
       int key;
       double data;

    public:
       SearchResult(Status s, Node n, int k, double d);
};

#endif
