set term postscript enhanced eps color ;
set key right;
set size .5,.5
#set xlabel  "Storage format"

set output 'ex1.eps';
set ylabel "File size (MB)"

set style data histogram
set style histogram cluster gap 1
set style fill pattern border
set boxwidth 0.9
plot 'sparse.dat' u ($2/1e6):xtic(1) ti col, '' u ($3/1e6) ti col
