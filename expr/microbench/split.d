#!/usr/sbin/dtrace -s
#pragma D option quiet

uint64_t rcount;
uint64_t wcount;
uint64_t rtime;
uint64_t wtime;
uint64_t begintime;
uint64_t nodecount;

riot$target:::btree-split-leaf
{
    printf("%d\n", arg0);
    @ = lquantize(arg0, 100, 300, 20);
}

