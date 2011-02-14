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


syscall::pread*:entry
/pid==$target/
/*pid==$target && dirname(fds[arg0].fi_pathname)=="/riot"*/
{
	self->ts = timestamp;
}

syscall::pwrite*:entry
/pid==$target/
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
	printf("%d %d %d %d %d\n", rcount, wcount, rtime, wtime, timestamp-begintime);
}

