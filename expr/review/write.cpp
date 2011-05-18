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

const int batchSize = 10000000;
Key_t keys[batchSize];

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

    for (i64 i=0; i<20000; ++i) {
        for (i64 j=0; j<20000; ++j) {
            MDCoord<2> c(i,j);
            array->put(c, 1.0);
        }
    }
	delete lin;
    delete array;
}
