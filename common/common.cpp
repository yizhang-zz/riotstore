#include "common.h"
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <map>
using namespace std;

static double R_ValueOfNA()
{
	/* The gcc shipping with RedHat 9 gets this wrong without
	 * the volatile declaration. Thanks to Marc Schwartz. */
	volatile ieee_double x;
	x.word[hw] = 0x7ff00000;
	x.word[lw] = 1954;
	return x.value;
}

double NA_DOUBLE = R_ValueOfNA();
// int NA_INT = INT32_MIN;

bool isNA(double x)
{
	if (isnan(x)) 
	{
		ieee_double y;
		y.value = x;
		return (y.word[lw] == 1954);
	}
	return false;
}

#ifdef RIOT_APPLE
int open_direct(const char *pathname, int flags)
{
	int fd = open(pathname, flags, 0660);
	if (fd < 0) return fd;
	int ret = fcntl(fd, F_NOCACHE, 1);
	if (ret == -1)
		Error("Cannot open %s in direct I/O mode", pathname);
	return fd;
}
#endif

#ifdef RIOT_SUN
int open_direct(const char *pathname, int flags)
{
	int fd = open(pathname, flags, 0660);
	if (fd < 0)
		return fd;
	directio(fd, DIRECTIO_ON);
    return fd;
}

PageImage allocPageImage(size_t num)
{
    return (PageImage) memalign(PAGE_SIZE, PAGE_SIZE*num);
}

#else
PageImage allocPageImage(size_t num)
{
    PageImage p;
    if (posix_memalign(&p, PAGE_SIZE, PAGE_SIZE*num) == 0)
        return p;
    return NULL;
}
#endif

void freePageImage(PageImage p)
{
    free(p);
}

/*
int compareKVPair(const void *a, const void *b)
{
    return (int)(((KVPair_t*)a)->key - ((KVPair_t*)b)->key);
}
*/
