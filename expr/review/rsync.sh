#!/bin/sh

rsync -avz --include='*.pdf' --exclude='*' . login:papers/riotstore/tr/figs/blocked
