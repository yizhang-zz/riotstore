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

for x in FWF LS LS_RAND LRU LG LG_RAND
do
	sed "s/\(batchMethod=\)\(.*\)/\1$x/g" $HOME/.riot > /tmp/.riot.tmp
	mv /tmp/.riot.tmp $HOME/.riot
	cat $HOME/.riot
for a in I R D
do
	for b in 2000 #4000 #8000
	do
		for c in B
		do
			echo "Running $a$b.in $c"
			./rw.d -c "./write $a$b.in $c" > /tmp/writerun.log
			RET=$?
			[ $RET -ne 0 ] && exit
			# use awk to calc sec from nanosec and drop the timestamp field
			# sed removes any blank line
			sort -n +4 /tmp/writerun.log | sed '/^$/d' | awk '{print $1,$2,$3,$4/1e9,$5/1e9}' > $1/$a$b$c-$x.log
			rm /tmp/writerun.log
		done
	done
done
done
