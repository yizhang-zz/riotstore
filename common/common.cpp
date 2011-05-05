#if defined(linux)
#define HAS_ODIRECT
#include <malloc.h>
/* Linux supports O_DIRECT in open(2) for direct I/O */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#endif // defined(linux)

#if defined(__FreeBSD__)
#define HAS_ODIRECT
#endif

#include <fcntl.h>
#include "common.h"
#include "Config.h"
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

int riot_open(const char *pathname, int flags)
{
    if (config->directio) {
#ifdef HAS_ODIRECT
        return open(pathname, (flags)|O_DIRECT, 0660);
#elif defined(__APPLE__)
        int fd = open(pathname, flags, 0660);
        if (fd < 0) return fd;
        int ret = fcntl(fd, F_NOCACHE, 1);
        if (ret == -1)
            Error("Cannot open %s in direct I/O mode", pathname);
        return fd;
#elif defined(sun)
        int fd = open(pathname, flags, 0660);
        if (fd < 0) return fd;
        directio(fd, DIRECTIO_ON);
        return fd;
#endif
    }
    else
        return open(pathname, flags, 0660);
}

PageImage allocPageImage(size_t num)
{
#if defined(sun)
    return (PageImage) memalign(PAGE_SIZE, PAGE_SIZE*num);
#else
    PageImage p;
    if (posix_memalign(&p, PAGE_SIZE, PAGE_SIZE*num) == 0)
        return p;
    return NULL;
#endif
}

void freePageImage(PageImage p)
{
    free(p);
}

