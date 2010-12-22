#ifndef PAGEREC_H
#define PAGEREC_H

#include "../common/common.h"

class BufferManager;

class PageRec
{
public:
	friend class BufferManager;
    char *image;
    PageRec *prev;
    PageRec *next;
    PID_t pid;
	u32 pinCount;
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

#endif
