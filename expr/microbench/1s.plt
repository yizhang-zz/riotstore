set term postscript enhanced eps color ;
set output 'io-1s.eps';
set key left;
set size .6,.6
set xlabel "# insertions (10^6)"
set ylabel "# I/Os"
plot \
"fwf/BS2000.log" using ($0/1000):2 ti "FWF" w l lw 2,\
"lru/BS2000.log" using ($0/1000):2 ti "LRU" w l lw 2,\
"ls/BS2000.log" using ($0/1000):2 ti "LS" w l lw 2,\
"lsrand/BS2000.log" using ($0/1000):2 ti "LSRAND" w l lw 2,\
"lg/BS2000.log" using ($0/1000):2 ti "LG" w l lw 2,\
"lgrand/BS2000.log" using ($0/1000):2 ti "LGRAND" w l lw 2

