#ifndef SPLITTER_H
#define SPLITTER_H

#include "../block/common.h"
#include "../block/block.h"

class Splitter {
   public:
      virtual Block* split(Block *n, int pos, Key key, Datum data) = 0;
};

class BSplitter : public Splitter {
      Block* split(Block *n, int pos, Key key, Datum data);
};

class MSplitter : public Splitter {
      Block* split(Block *n, int pos, Key key, Datum data);
};

class RSplitter : public Splitter {
      Block* split(Block *n, int pos, Key key, Datum data);
};

class SSplitter : public Splitter {
      Block* split(Block *n, int pos, Key key, Datum data);
};

class TSplitter : public Splitter {
      Block* split(Block *n, int pos, Key key, Datum data);
};

#endif
