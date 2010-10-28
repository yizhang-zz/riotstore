#!/usr/sbin/dtrace -s
#pragma D option quiet

uint64_t rcount;
uint64_t wcount;
uint64_t iotime;
uint64_t begintime;
uint64_t leafcount;

BEGIN
{
	begintime = timestamp;
	freq = 1000;
	self->timer = 0;
}

riot$target:::btree-locate-begin
{
	self->locater = 0;
	self->locatew = 0;
}

riot$target:::btree-locate-end
{
	/*@locater[arg0, self->locater] = count();
	@locatew[arg0, self->locatew] = count();*/
	self->locater = self->locatew = 0;
}

riot$target:::btree-put
/ self->timer > 0 /
{
	self->timer--;
	/*self->a++;*/
}

riot$target:::btree-put
/ self->timer == 0 /
{
	self->timer = freq;
	/*self->a++;*/
	/*printf("%d %d %d %d %d %d\n", leafcount, rcount, wcount, iotime, timestamp-begintime, timestamp);*/
}

riot$target:::btree-split-leaf
{
	leafcount++;
	/*@leaf["a"] = count();*/
}

syscall::read*:entry
/pid==$target && fds[arg0].fi_pathname=="/riot/mb.1"/
{
	self->ts = timestamp;
}

syscall::write*:entry
/pid==$target && fds[arg0].fi_pathname=="/riot/mb.1"/
{
	self->ts = timestamp;
}

syscall::read*:return
/ self->ts /
{
	rcount++;
	self->locater++;
	iotime += timestamp - self->ts;
	@distr = quantize(timestamp-self->ts);
	self->ts = 0;
}

syscall::write*:return
/ self->ts /
{
	wcount++;
	self->locatew++;
	iotime += timestamp - self->ts;
	@distw = quantize(timestamp-self->ts);
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

END
{
	printf("%d %d %d %d %d %d\n", leafcount, rcount, wcount, iotime, timestamp-begintime, timestamp);
	printa(@distr);
	printa(@distw);
}

