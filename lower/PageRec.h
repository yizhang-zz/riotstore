#ifndef PAGEREC_H
#define PAGEREC_H

#include "../common/common.h"
#include <iostream>
#include <boost/pool/pool.hpp>

class BufferManager;

class PageRec
{
public:
	friend class BufferManager;
    friend std::ostream & operator<< (std::ostream &, const PageRec &);

    char *image;
    PageRec *prev;
    PageRec *next;
    PID_t pid;
	int pinCount;
    bool dirty;
    PageRec()
    {
        reset();
    }

    void reset()
    {
        pid = INVALID_PID;
        dirty = false;
        pinCount = 0;
        prev = next = NULL;
    }
};

class Page
{
private:
	PageRec *prec;
	BufferManager *buffer;
public:
	Page(PageRec *p, BufferManager *b);
	~Page();
	void markDirty();
	void flush();
	PID_t getPid() const;
	char* getImage() const;
};

struct PageDealloc
{
    PageDealloc(boost::pool<> &p) : pool(p) { }
    PageDealloc(const PageDealloc &other) : pool(other.pool) { }
    void operator() (Page *p) { p->~Page(); pool.free(p); }
private:
    boost::pool<> &pool;
};

#endif
