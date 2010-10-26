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
	char fileName[] = "/riot/mb.1";
	//srand(12874938);
	unsigned int tm;
	if (argc >= 5)
		tm = atoi(argv[4]);
	else
		tm = time(NULL);
	srand(tm);
	cerr<<"seed = "<<tm<<endl;
	if (argc < 4) {
		cout<<"Usage: "<<argv[0]<<" <splitter type> <insertion sequence> <size> [rand seed]"<<endl;
		cout<<"splitter type: M,B,R,S,T"<<endl
			<<"insertion sequence: S(sequential), D(strided), R(random)"<<endl
			<<"size: #rows/cols of the square matrix"<<endl
			<<"rand seed: seed, unsigned int"<<endl;
			//<<"sparsity: e.g., .4 means 40% of the elements are non-zero"<<endl;
		return 0;
	}

	char splitterType = argv[1][0];
	char seqType = argv[2][0];
	int size = atoi(argv[3]);
	//double sparsity = atof(argv[4]);

	InternalSplitter *isp = new MSplitter<PID_t>();
	LeafSplitter *lsp = NULL;
	switch(splitterType) {
	case 'M':
		lsp = new MSplitter<Datum_t>();
		break;
	case 'B':
#ifdef DISABLE_DENSE_LEAF
		lsp = new BSplitter<Datum_t>(config->sparseLeafCapacity);
#else
		lsp = new BSplitter<Datum_t>(config->denseLeafCapacity);
#endif
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

	int total = size*size;
	BTree *tree = new BTree(fileName, total, lsp, isp);
	Key_t *keys = new Key_t[total];
	int i,j;
	switch(seqType) {
	case 'S':
		for (i=0; i<total; ++i) {
			keys[i] = i;
		}
		break;
	case 'D':
		for (i=0; i<size; ++i) 
			for (j=0; j<size; ++j) 
				keys[i*size+j] = j*size+i;
		break;
	case 'R':
		for (i=0; i<total; ++i) 
			keys[i] = i;
		permute(keys, total);
		break;
	}

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
	for (i=0; i<total; ++i) {
		tree->put(keys[i], 1.0);
		//if (i%interval != 0)
		//	continue;

		/*
		log.width(10);
		log<<tree->getNumLeaves();
		log.width(10);
		log<<PagedStorageContainer::readCount+PagedStorageContainer::writeCount;
		log.width(12);
		log<<PagedStorageContainer::accessTime;
		TIMESTAMP(stop);
		log.width(12);
		log<<stop-start<<endl;
		*/
	}
	delete tree;
	/*
		log.width(10);
		log<<tree->getNumLeaves();
		log.width(10);
		log<<PagedStorageContainer::readCount+PagedStorageContainer::writeCount;
		log.width(12);
		log<<PagedStorageContainer::accessTime;
		TIMESTAMP(stop);
		log.width(12);
		log<<stop-start<<endl;
*/
		delete keys;
}
