#!/usr/sbin/dtrace -qs

uint64_t x;
int t;

syscall::pwrite*:entry
/ dirname(fds[arg0].fi_pathname) == "/riot" /
{
printf("%d 0 %d\n", x, (arg3)/8192);
}

syscall::pread*:entry
/ dirname(fds[arg0].fi_pathname) == "/riot" /
{
printf("%d 1 %d\n", x, (arg3)/8192);
}

riot$target:::btree-put
{
    x++;
}

/*
riot$target:::bm-read,
riot$target:::bm-alloc
/ t /
{
    bmread++;
    t--;
}

riot$target:::bm-read,
riot$target:::bm-alloc
/ t == 0 /
{
    bmread++;
    t = 100;
    printf("%d %d\n", x, bmread);
}
*/
