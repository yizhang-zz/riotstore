#include "utils.h"

SearchResult::SearchResult(Status s, Node n, int k, double d) {
    status = s;
    node = n;
    key = k;
    data = d;
}
