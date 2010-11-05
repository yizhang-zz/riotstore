dir="new/";
seq="D";
scale="2000";
split="B";
batch='FWF LS LS_RAND LG LG_RAND';
#batch='FWF LRU LS LS_RAND LG LG_RAND';

set term postscript enhanced eps color ;
set key left;
set size .6,.6
set xlabel "# insertions (10^6)"

set output dir.seq.'nleaves.eps';
set ylabel "# leaf nodes"
plot for [b in batch] dir.seq.scale.split."-".b.".log" usi ($0/1000):1 ti b w l lw 2

set output dir.'read.eps';
set ylabel "# reads"
plot for [b in batch] dir.seq.scale.split."-".b.".log" usi ($0/1000):2 ti b w l lw 2

set output dir.'write.eps';
set ylabel "# writes"
plot for [b in batch] dir.seq.scale.split."-".b.".log" usi ($0/1000):3 ti b w l lw 2

set output dir.seq.'io.eps';
set ylabel "# I/Os"
plot for [b in batch] dir.seq.scale.split."-".b.".log" usi ($0/1000):($2+$3) ti b w l lw 2


set output dir.seq.'iotime.eps';
set ylabel "I/O time (s)"
plot for [b in batch] dir.seq.scale.split."-".b.".log" usi ($0/1000):4 ti b w l lw 2

set output dir.seq.'totaltime.eps';
set ylabel "total time (s)"
plot for [b in batch] dir.seq.scale.split."-".b.".log" usi ($0/1000):5 ti b w l lw 2

