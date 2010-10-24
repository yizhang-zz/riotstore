#!/bin/sh

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

for c in 1000 #2000 #4000 8000
do
	for b in S D R
	do
		for a in M B R S T
		do
			echo "Running $a $b $c"
			./rw.d -c "./mb $a $b $c" > tmp.log
			# use awk to calc sec from nanosec and drop the timestamp field
			# sed removes any blank line
			sort -n +4 tmp.log | sed '/^$/d' | awk '{print $1,$2,$3/1e9,$4/1e9}' > $1/$a$b$c.log
			rm tmp.log
		done
	done
done
