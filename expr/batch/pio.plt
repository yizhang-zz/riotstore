#!/usr/bin/env gnuplot
set term png
set output 'w.png'
#plot '/tmp/m.log' usi 1:2 w l
plot '/tmp/a.log' usi 1:($2==0?$3:1/0) w dots

set output 'r.png'
plot '/tmp/a.log' usi 1:($2==1?$3:1/0) w dots

set output 'b1w.png'
#plot '/tmp/b.log' usi 1:2 w l
plot '/tmp/b1.log' usi 1:($2==0?$3:1/0) w dots

set output 'b1r.png'
plot '/tmp/b1.log' usi 1:($2==1?$3:1/0) w dots
