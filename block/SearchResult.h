#ifndef SEARCH_RESULT_H
#define SEARCH_RESULT_H

enum SearchStatus {FOUND, NONE};

class SearchResult
{
	public:
	SearchStatus status;
	BlockNo blkno;
	Key key;
	Datum datum;

	SearchResult(SearchStatus s, BlockNo b, Key k, Datum d)
	{
		status = s;
		blkno = b;
		key = k;
		datum = d;
	}
};

#endif


