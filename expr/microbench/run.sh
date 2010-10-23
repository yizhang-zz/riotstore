#!/bin/sh

for c in 1000 2000 #4000 8000
do
	for b in S D R
	do
		for a in M B R S T
		do
			echo "Running $a $b $c"
			./mb $a $b $c
		done
	done
done
