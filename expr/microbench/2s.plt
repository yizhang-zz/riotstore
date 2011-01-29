set term postscript enhanced eps color ;
set output 'iotime-s.eps';
set key left;
set size .6,.6
set xlabel "# insertions (10^6)"
set ylabel "# I/O time (s)"
plot \
"fwf/BS2000.log" using ($0/1000):3 ti "FWF" w l lw 2,\
"lru/BS2000.log" using ($0/1000):3 ti "LRU" w l lw 2,\
"ls/BS2000.log" using ($0/1000):3 ti "LS" w l lw 2,\
"lsrand/BS2000.log" using ($0/1000):3 ti "LSRAND" w l lw 2,\
"lg/BS2000.log" using ($0/1000):3 ti "LG" w l lw 2,\
"lgrand/BS2000.log" using ($0/1000):3 ti "LGRAND" w l lw 2

