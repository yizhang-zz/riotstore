#include "PageRec.h"
#include "BufferManager.h"

std::ostream & operator<< (std::ostream &out, const PageRec &pr)
{
    out<<pr.pid;
    if (pr.dirty)
        out<<"*";
    out<<"("<<pr.pinCount<<") ";
    return out;
}

/*
Page::Page(PageRec *p, BufferManager *b): prec(p), buffer(b)
{
	buffer->pinPage(prec);
}

Page::~Page()
{
	buffer->unpinPage(prec);
}

void Page::markDirty()
{
	buffer->markPageDirty(prec);
}

void Page::flush()
{
	buffer->flushPage(prec);
}

PID_t Page::getPid() const
{
	return prec->pid;
}

char* Page::getImage() const
{
	return prec->image;
}
*/
