#ifndef UTILS_H
#define UTILS_H

#include "../block/common.h"

enum SearchStatus { NONE, FOUND};

class SearchResult {
    public:
       SearchStatus status;
       BlockNo blkno;
       Key key;
       Datum datum;

    public:
       SearchResult(SearchStatus s, BlockNo n, Key k, Datum d);
};

#endif
