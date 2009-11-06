#!/bin/bash

make clean
make all
g++ TestBitmapPagedFile.o BitmapPagedFile.o -o test
./test
