#include <time.h>
#include <iostream>
#include <fstream>
#include "common.h"
#include "Config.h"
#include "Btree.h"

using namespace std;
using namespace Btree;

int main(int argc, char **argv)
{
	const int required = 3;
	char fileName[] = "/riot/mb.1";
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
			<<"input file: e.g., S2000.in (sequential sequence of 2k x 2k matrix"<<endl
			<<"splitter type: M,B,R,S,T"<<endl
			<<"rand seed: seed, unsigned int"<<endl;
			//<<"sparsity: e.g., .4 means 40% of the elements are non-zero"<<endl;
		return 0;
	}

	//char seqType = argv[1][0];
	//int size = atoi(argv[2]);
	char *filename = argv[1];
	char splitterType = argv[2][0];

	int infile = open(filename, O_RDONLY);

    /*
	InternalSplitter *isp = new MSplitter<PID_t>();
	LeafSplitter *lsp = NULL;
	switch(splitterType) {
	case 'M':
		lsp = new MSplitter<Datum_t>();
		break;
	case 'B':
		lsp = new BSplitter<Datum_t>(config->BSplitterBoundary());
		break;
	case 'R':
		lsp = new RSplitter<Datum_t>();
		break;
	case 'S':
		lsp = new SSplitter<Datum_t>();
		break;
	case 'T':
		lsp = new TSplitter<Datum_t>(config->TThreshold);
		break;
	}
    */
	Key_t size = atoi(filename+1);
	Key_t total = size*size;
	BTree *tree = new BTree(fileName, total, splitterType, 'M', config->useDenseLeaf);

	/*
	char logFile[100];
	logFile[0] = splitterType;
	logFile[1] = seqType;
	sprintf(logFile+2, "%d.log", size);

	ofstream log(logFile, ios_base::out|ios_base::trunc);
	log<<"#leaves\t#IO\tIO time\ttotal time"<<endl;
	PagedStorageContainer::resetPerfCounts();
	TIMESTAMP(start)
	int interval = 100;
	*/
	const int batchSize = 1000;
	Key_t keys[batchSize];
	while (true) {
		ssize_t c = read(infile, keys, sizeof(keys));
		int count = c/sizeof(Key_t);
		for (int i=0; i<count; ++i) {
			tree->put(keys[i], 1.0);
		}
		if (count < batchSize)
			break;
	}
	close(infile);
	delete tree;
	//delete isp;
	//delete lsp;
}
