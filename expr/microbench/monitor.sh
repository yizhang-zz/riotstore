#!/bin/sh
while [ `pgrep run.sh` ]; do
    sleep 10
    echo "waiting for run.sh to finish"
done
#./run1ad 40k

BUF=51000
sed "s/\(btreeBufferSize=\)\(.*\)/\1$BUF/g" $HOME/.riot > /tmp/.riot.tmp
mv /tmp/.riot.tmp $HOME/.riot
./run.sh 20k.$BUF
