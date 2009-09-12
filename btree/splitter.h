#ifndef SPLITTER_H
#define SPLITTER_H

class Splitter {
   public:
      virtual Node split(Node n, int pos, int key, double data) = 0;
};

class BSplitter : public Splitter {
      Node split(Node n, int pos, int key, double data);
};

class MDSplitter : public Splitter {
      Node split(Node n, int pos, int key, double data);
};

class MSplitter : public Splitter {
      Node split(Node n, int pos, int key, double data);
};

class RSplitter : public Splitter {
      Node split(Node n, int pos, int key, double data);
};

class SSplitter : public Splitter {
      Node split(Node n, int pos, int key, double data);
};

class TBSplitter : public Splitter {
      Node split(Node n, int pos, int key, double data);
};
class TSplitter : public Splitter {
      Node split(Node n, int pos, int key, double data);
};
class ZSplitter : public Splitter {
      Node split(Node n, int pos, int key, double data);
};

#endif
