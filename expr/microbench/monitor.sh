#!/bin/sh
while [ `pgrep run.sh` ]; do
    sleep 5
    echo "waiting for run.sh to finish"
done
./run1.sh Final1_
# generate S4000X-0-NONE.bin files for read
./runs.sh temps
# read test
./run-read Read1_
