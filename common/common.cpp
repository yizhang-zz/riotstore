#include "common.h"
#include <math.h>
#include <stdio.h>
#include <stdarg.h>

void riot_error(const char *s, ...)
{
	va_list(arg);
	va_start(arg, s);
	vfprintf(stderr, s, arg);
	va_end(arg);
}

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

#ifdef RIOT_BSD
int open_direct_bsd(const char *pathname, int flags)
{
	int fd = open(pathname, flags, 0660);
	if (fd < 0) return fd;
	int ret = fcntl(fd, F_NOCACHE, 1);
	if (ret == -1)
		riot_error("Cannot open %s in direct I/O mode.\n", pathname);
	return fd;
}
PageImage allocPageImage(size_t num)
{
    PageImage p;
    if (posix_memalign(&p, PAGE_SIZE, PAGE_SIZE*num))
        return p;
    return NULL;
}

void freePageImage(PageImage p)
{
    free(p);
}
#endif

#ifdef RIOT_SUN
int open_direct_sol(const char *pathname, int flags)
{
	int fd = open(pathname, flags, 0660);
	if (fd < 0)
		return fd;
	return directio(fd, DIRECTIO_ON);
}
PageImage allocPageImage(size_t num)
{
    return (PageImage) memalign(PAGE_SIZE, PAGE_SIZE*num);
}

void freePageImage(PageImage p)
{
    free(p);
}
#endif

#ifdef RIOT_LINUX
PageImage allocPageImage(size_t num)
{
    return (PageImage) memalign(PAGE_SIZE, PAGE_SIZE*num);
}

void freePageImage(PageImage p)
{
    free(p);
}
#endif
