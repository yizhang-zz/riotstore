#!/usr/sbin/dtrace -s
#pragma D option quiet

uint64_t rcount;
uint64_t wcount;
uint64_t rtime;
uint64_t wtime;
uint64_t begintime;
uint64_t nodecount;

BEGIN
{
	begintime = timestamp;
	freq = 1000;
	self->timer = 0;
}

riot$target:::dma-put,
riot$target:::btree-put
/ self->timer > 0 /
{
	self->timer--;
}

riot$target:::dma-put,
riot$target:::btree-put
/ self->timer == 0 /
{
	self->timer = freq;
	printf("%d %d %d %d %d %d\n", nodecount, rcount, wcount, rtime, wtime, timestamp-begintime);
}

riot$target:::btree-new-internal,
riot$target:::btree-split-leaf,
riot$target:::dma-new-block
{
    nodecount++;
}

syscall::pread:entry,
syscall::pwrite:entry
/ pid==$target /
{
	self->ts = timestamp;
}

syscall::pread*:return
/ self->ts /
{
	rcount++;
	rtime += timestamp - self->ts;
	self->ts = 0;
}

syscall::pwrite*:return
/ self->ts /
{
	wcount++;
	wtime += timestamp - self->ts;
	self->ts = 0;
}

END
{
	printf("%d %d %d %d %d %d\n", nodecount, rcount, wcount, rtime, wtime, timestamp-begintime);
}

