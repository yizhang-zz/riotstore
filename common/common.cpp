#include "common.h"
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


void Error(const char *s, ...)
{
	va_list(arg);
	va_start(arg, s);
    char *buf = new char[strlen(s)+16];
    sprintf(buf, "[ERROR] %s\n", s);
	vfprintf(stderr, buf, arg);
	va_end(arg);
    delete[] buf;
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
    return fd;
	if (fd < 0)
		return fd;
	return directio(fd, DIRECTIO_ON);
}

PageImage allocPageImage(size_t num)
{
    return (PageImage) memalign(PAGE_SIZE, PAGE_SIZE*num);
}

#else
PageImage allocPageImage(size_t num)
{
    PageImage p;
    if (!posix_memalign(&p, PAGE_SIZE, PAGE_SIZE*num))
        return p;
    return NULL;
}
#endif

void freePageImage(PageImage p)
{
    free(p);
}

#ifdef DEBUG
void debug(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char *buf = new char[strlen(format)+12];
    sprintf(buf, "[DEBUG] %s\n", format);
    vfprintf(stderr, buf, args);
    va_end(args);
    delete[] buf;
}
#endif

int compareKVPair(const void *a, const void *b)
{
    return (int)(((KVPair_t*)a)->key - ((KVPair_t*)b)->key);
}
