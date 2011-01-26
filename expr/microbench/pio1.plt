#!/usr/bin/env gnuplot
set term png

set output 'm.png'
plot '/tmp/m.log' usi 1:($2==0?$3:1/0) w p,\
'/tmp/m.log' usi 1:($2==1?$3:1/0) w p

set output 'b.png'
plot '/tmp/b.log' usi 1:($2==0?$3:1/0) w p,\
'/tmp/b.log' usi 1:($2==1?$3:1/0) w p
