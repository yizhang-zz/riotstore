export RIOT_DMA_BUFFER=10
export RIOT_BTREE_BUFFER=10

./test-mat-write

#for i in {1..2}

#do
#/usr/bin/time ./test-elemops 
#done


#for i in {1..10}
#do
#g++ -DPROFILING  `pkg-config --cflags-only-I apr-1` `pkg-config --libs apr-1` -o testblas -g blas.cpp ../libriot_store.so -lblas -lm
#/usr/bin/time -a -o blas.bin ./testblas
#done
