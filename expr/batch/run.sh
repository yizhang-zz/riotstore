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
useDense=`sed -n "s/useDenseLeaf=\([01]\)/\1/p" $HOME/.riot`
echo "useDense=$useDense"
echo

for x in LPP LP ALL LG LGP 
do
	sed "s/\(batchMethod=\)\(.*\)/\1$x/g" $HOME/.riot > /tmp/.riot.tmp
	mv /tmp/.riot.tmp $HOME/.riot
    echo ">> .riot"
	cat $HOME/.riot
    echo "<<"
    echo

# workload
for a in R
do
    # matrix size
	for b in 4000
	do
        # splitting strategy
		for c in B 
		do
			echo "Running with input $a$b , splitting strategy $c"
            output=$a$b$c-$useDense-$x
            echo "output will be named $output"
			./rw.d -c "./write $a$b $c" > /tmp/writerun.log
			# use awk to calc sec from nanosec and drop the timestamp field
			# sed removes any blank line
			sort -n +5 /tmp/writerun.log | sed '/^$/d' | awk '{print $1,$2,$3,$4/1e9,$5/1e9,$6/1e9}' > $1/$output.log
			rm /tmp/writerun.log
            cp /riot/mb /riot/$output.bin
		done
	done
done
done
