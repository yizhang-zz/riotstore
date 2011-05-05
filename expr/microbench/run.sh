#!/bin/sh

# File names consists of the following parts:
# insertion seq: S|D|R
# insertion seq scale: number
# splitting strategy: M|B|R|S|T
# Batching method: NONE, FWF, ...
# Batch buffer size: number

if test -z "$1"
then
	echo "Error: Provide result dir name as argument!"
	exit
fi

# Should allow dir to exist, in which case results are added to that dir
if [[ -f "$1" ]]
then
	echo "Error: There exists a file named '$1' !"
	exit
fi

if [[ ! -d "$1" ]]
then
	mkdir $1
fi

# whether to use dense leaf format
# read from $HOME/.riot conf file
#useDense=`sed -n "s/useDenseLeaf=\([01]\)/\1/p" $HOME/.riot`

for useDense in 0 1; do
sed "s/\(useDenseLeaf=\)\(.*\)/\1$useDense/g" $HOME/.riot > /tmp/.riot.tmp
mv /tmp/.riot.tmp $HOME/.riot
echo "# useDense=$useDense"

for x in NONE #FWF LS LS_RAND LRU LG LG_RAND
do
	sed "s/\(batchMethod=\)\(.*\)/\1$x/g" $HOME/.riot > /tmp/.riot.tmp
	mv /tmp/.riot.tmp $HOME/.riot
    echo ">> .riot"
	cat $HOME/.riot
    echo "<<"
    echo

# workload
for a in S I D 
do
    # matrix size
	for b in 20000
	do
        # splitting strategy
		for c in M A R T
		do
            echo "write test: $a$b , splitting strategy $c (D means DMA)"
            output=$a$b$c-$useDense-$x
            echo "output will be named $output"
			./rw.stp -c "./write $a$b $c /riot" > /tmp/writerun.log
			# use awk to calc sec from nanosec and drop the timestamp field
			# sed removes any blank line
			sort -n -k 6,6 /tmp/writerun.log | sed '/^$/d' | awk '{print $1,$2,$3,$4/1e9,$5/1e9,$6/1e9}' > $1/$output.log
			rm /tmp/writerun.log
            #cp /riot/mb /riot/$output.bin

            continue #do not need read results
            
            case "$c$useDense" in
                M0|A1|D1) echo "reading $output"
                    ln -sf /riot/mb /riot/$output.bin
                    ./read.stp -c "./read /riot/$output.bin R" > $1/$output-R.log
                    rm /riot/$output.bin
                    ;;
            esac
		done
	done
done
done
done
