#!/usr/sbin/dtrace -s
#pragma D option quiet

BEGIN
{
	iocount = 0;
	iotime = 0;
	begintime = timestamp;
	leafcount = 0;
	freq = 1000;
	self->timer = 0;
}

END
{
	printf("%d %d %d %d %d\n", leafcount, iocount, iotime, timestamp-begintime, timestamp);
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
	printf("%d %d %d %d %d\n", leafcount, iocount, iotime, timestamp-begintime, timestamp);
}

riot$target:::btree-split-leaf
{
	leafcount++;
	/*@leaf["a"] = count();*/
}

syscall::read*:entry,
syscall::write*:entry
/pid==$target && fds[arg0].fi_pathname=="/riot/mb.1"/
{
	self->ts = timestamp;
}

syscall::read*:return,
syscall::write*:return
/ self->ts /
{
	iocount++;
	iotime += timestamp - self->ts;
	self->ts = 0;
}
