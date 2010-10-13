#ifndef PAGEREC_H
#define PAGEREC_H

#include "../common/common.h"

struct PageRec
{
    PID_t pid;
    void *image;
    //void *unpacked;
	uint32_t pinCount;
    bool dirty;
    PageRec *prev;
    PageRec *next;

    PageRec()
    {
        reset();
    }

    void reset()
    {
        pid = INVALID_PID;
        dirty = false;
        pinCount = 0;
        //unpacked = NULL;
        prev = next = NULL;
    }
};
#endif
