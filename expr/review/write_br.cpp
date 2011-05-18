#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include "common.h"
#include "Config.h"
#include "BlockBased.h"
#include "MDArray.h"

using namespace std;
using namespace Btree;

#define S 14U
#define N (1U<<S)

static const unsigned char BitReverseTable256[256] = 
{
#   define R2(n)     n,     n + 2*64,     n + 1*64,     n + 3*64
#   define R4(n) R2(n), R2(n + 2*16), R2(n + 1*16), R2(n + 3*16)
#   define R6(n) R4(n), R4(n + 2*4 ), R4(n + 1*4 ), R4(n + 3*4 )
    R6(0), R6(2), R6(1), R6(3)
};

// Option 1:
static inline unsigned bitreverse(unsigned v)
{
	unsigned c = (BitReverseTable256[v & 0xff] << 24) | 
    (BitReverseTable256[(v >> 8) & 0xff] << 16) | 
    (BitReverseTable256[(v >> 16) & 0xff] << 8) |
    (BitReverseTable256[(v >> 24) & 0xff]);
	return c >> (32 - S);
}

// Option 2:
/*
unsigned char * p = (unsigned char *) &v;
unsigned char * q = (unsigned char *) &c;
q[3] = BitReverseTable256[p[0]]; 
q[2] = BitReverseTable256[p[1]]; 
q[1] = BitReverseTable256[p[2]]; 
q[0] = BitReverseTable256[p[3]];
*/

// write in bit reversed order
int main(int argc, char **argv)
{
	const int required = 2;
	char fileName[100] = "/riot/mb";
	unsigned int tm;
	if (argc >= required+1)
		tm = atoi(argv[required]);
	else
		tm = time(NULL);
	srand(tm);
	cerr<<"seed = "<<tm<<endl;
	if (argc < required) {
		cerr<<"Usage: "<<argv[0]<<"<splitter type>"<<endl
			<<"splitter type: M,A,D"<<endl;
		return 0;
	}

	char splitterType = argv[1][0];

    i64 arraydims[] = {20000,20000};
    i64 blockdims[] = {31,31};
    u8 orders[] = {0, 1};
    Linearization<2> *lin = new BlockBased<2>(arraydims, blockdims, orders,
                                              orders);
    MDArray<2> *array;
    StorageParam sp;
    sp.fileName = fileName;
    sp.intSp = 'M';
    sp.leafSp = splitterType;
    if (splitterType == 'D') {
        // dma
        sp.type = DMA;
    }
    else if (splitterType == 'M') {
        sp.type = BTREE;
        sp.useDenseLeaf = false;
    }
    else if (splitterType == 'A') {
        sp.type = BTREE;
        sp.useDenseLeaf = true;
    }
    else {
        cerr<<"wrong splitter type"<<endl;
        exit(1);
    }
    
    array = new MDArray<2>(&sp, MDCoord<2>(arraydims), lin);

	unsigned i,j;
    for (i=0; i<N; ++i) {
        for (j=0; j<N; ++j) {
            MDCoord<2> c(i, bitreverse(j));
            array->put(c, 1.0);
        }
    }
	delete lin;
    delete array;
}
