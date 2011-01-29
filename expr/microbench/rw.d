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

/*
riot$target:::btree-locate-begin
{
	self->locater = 0;
	self->locatew = 0;
}

riot$target:::btree-locate-end
{
}
*/

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

syscall::pread*:entry
/pid==$target && basename(fds[arg0].fi_pathname)=="mb"/
{
	self->ts = timestamp;
}

syscall::pwrite*:entry
/pid==$target && basename(fds[arg0].fi_pathname)=="mb"/
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

/* exit with the same code if segfault happens */
/*
proc:::signal-send
/args[2]==SIGSEGV && pid==$target/
{
	exit(arg0);
}
*/

/*
io:::start
/ args[2]->fi_name=="mb.1" && args[0]->b_flags & B_READ /
{
*/
	/*
	@io[args[0]->b_flags & B_READ, args[0]->b_bcount] = count();
	@iob[args[0]->b_flags & B_READ] = quantize(args[0]->b_lblkno);
	*/
/*
	rcount++;
	self->ts = timestamp;
}
*/

/*
io:::start
/ args[2]->fi_name=="mb.1" && args[0]->b_flags & B_WRITE /
{
	wcount++;
	self->ts = timestamp;
	printf("start %d\n", timestamp);
}

io:::done
{
	printf("done %d\n", timestamp);
	rwtime += timestamp - self->ts;
	self->ts = 0;
}
*/

END
{
	printf("%d %d %d %d %d %d\n", nodecount, rcount, wcount, rtime, wtime, timestamp-begintime);
}

