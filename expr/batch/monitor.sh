#!/bin/sh
while [ `pgrep run.sh` ]; do
#if [ `pgrep run.sh` ]; then 
    sleep 5
    echo "waiting for run.sh to finish"
#else
#    ./run1.sh batch6
#    break
#fi
done
./run1.sh batch6
