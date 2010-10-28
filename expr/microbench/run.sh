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

if [[ -d "$1" || -f "$1" ]]
then
	echo "Error: Dir or file $1 exists!"
	exit
fi

mkdir $1

for x in FWF LRU LS LS_RAND LG LG_RAND
do
	sed "s/\(batchMethod=\)\(.*\)/\1$x/g" $HOME/.riot > /tmp/.riot.tmp
	mv /tmp/.riot.tmp $HOME/.riot
	cat $HOME/.riot
for a in D
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
			sort -n +5 /tmp/writerun.log | sed '/^$/d' | awk '{print $1,$2,$3,$4/1e9,$5/1e9}' > $1/$a$b$c-$x.log
			rm /tmp/writerun.log
		done
	done
done
done
