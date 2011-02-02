#ifndef PAGEREC_H
#define PAGEREC_H

#include "../common/common.h"
#include "lower/BufferManager.h"
//#include <iostream>
//#include <boost/pool/pool.hpp>

//class BufferManager;

class PageRec
{
public:
	friend class BufferManager;
    friend std::ostream & operator<< (std::ostream &, const PageRec &);

	BufferManager *buffer;
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

    void markDirty()
    {
        buffer->markPageDirty(this);
    }

    void flush()
    {
        buffer->flushPage(this);
    }

    void unpin()
    {
        buffer->unpinPage(this);
    }

    void pin()
    {
        buffer->pinPage(this);
    }

	PID_t getPid() const
    {
        return pid;
    }

	char* getImage() const
    {
        return image;
    }

    int getIndex() const
    {
        return index;
    }

private:
    int index;
};

/*
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
*/

struct PageDealloc
{
    //PageDealloc(boost::pool<> &p) : pool(p) { }
    //PageDealloc(const PageDealloc &other) : pool(other.pool) { }
    void operator() (PageRec *p) { p->unpin(); }
private:
    //boost::pool<> &pool;
};

#endif
