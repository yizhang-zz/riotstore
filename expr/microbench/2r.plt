set term postscript enhanced eps color ;
set output 'iotime-2r.eps';
set key left;
set size .6,.6
set xlabel "# insertions (10^6)"
set ylabel "# I/O time (s)"
plot \
"fwf/BR2000.log" using ($0/1000):3 ti "FWF" w l lw 2,\
"lru/BR2000.log" using ($0/1000):3 ti "LRU" w l lw 2,\
"ls/BR2000.log" using ($0/1000):3 ti "LS" w l lw 2,\
"lsrand/BR2000.log" using ($0/1000):3 ti "LSRAND" w l lw 2,\
"lg/BR2000.log" using ($0/1000):3 ti "LG" w l lw 2,\
"lgrand/BR2000.log" using ($0/1000):3 ti "LGRAND" w l lw 2

