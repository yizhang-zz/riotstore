#include <stdio.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include "common.h"
#include "Config.h"
#include "Btree.h"
#include "DirectlyMappedArray.h"

using namespace std;
using namespace Btree;

int main(int argc, char **argv)
{
	const int required = 3;
	char fileName[100] = "/riot/mb";
	//srand(12874938);
	unsigned int tm;
	if (argc >= required+1)
		tm = atoi(argv[required]);
	else
		tm = time(NULL);
	srand(tm);
	cerr<<"seed = "<<tm<<endl;
	if (argc < required) {
		cerr<<"Usage: "<<argv[0]<<" <input file> <splitter type> [rand seed]"<<endl
			//<<"insertion sequence: S(sequential), D(strided), R(random)"<<endl
			//<<"size: #rows/cols of the square matrix"<<endl
			<<"input file: e.g., S2000 (sequential sequence of 2k x 2k matrix"<<endl
			<<"splitter type: M,B,R,S,T"<<endl
			<<"rand seed: seed, unsigned int"<<endl;
			//<<"sparsity: e.g., .4 means 40% of the elements are non-zero"<<endl;
		return 0;
	}

	//char seqType = argv[1][0];
	//int size = atoi(argv[2]);
	char *infilename = argv[1];
	char splitterType = argv[2][0];

	int infile = open(infilename, O_RDONLY);

    // if infilename contains 'x', then it contains both dimensions of the
    // matrix; otherwise the two dims are equanl and only one is provided
    char x[] = "x";
    Key_t total;
    if (strstr(infilename, x)) {
        Key_t d1,d2;
        sscanf(infilename+1, "%lux%lu", &d1, &d2);
        total = d1*d2;
    }
    else {
        total = atoi(infilename+1);
        total *= total;
    }
    LinearStorage *ls;
    if (splitterType == 'D') {
        // dma
        ls = new DirectlyMappedArray(fileName, total);
    }
    else if (splitterType == 'M') 
        ls = new BTree(fileName, total, splitterType, 'M', 0);
    else if (splitterType == 'A')
        ls = new BTree(fileName, total, splitterType, 'M', 1);
    else {
        cerr<<"wrong splitter type"<<endl;
        exit(1);
    }

	const int batchSize = 1000000;
	Key_t keys[batchSize];
    //for (int j=0; j<1002; ++j) {
	while (true) {
		ssize_t c = read(infile, keys, sizeof(keys));
		int count = c/sizeof(Key_t);
		for (int i=0; i<count; ++i) {
			ls->put(keys[i], 1.0);
		}
		if (count < batchSize)
			break;
	}
	close(infile);
	delete ls;
}
