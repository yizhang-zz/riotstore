
export RIOT_DMA_BUFFER=10
export RIOT_BTREE_BUFFER=10
for i in {1..10}
do
g++ `pkg-config --cflags-only-I apr-1` `pkg-config --libs apr-1` -o testelemops elemops.cpp ../libriot_store.a
/usr/bin/time -a -o elemops.bin ./testelemops 
done

for i in {1..10}
do
g++ `pkg-config --cflags-only-I apr-1` `pkg-config --libs apr-1` -o testblas -g blas.cpp ../libriot_store.a -lblas -lm
/usr/bin/time -a -o blas.bin ./testblas
done
