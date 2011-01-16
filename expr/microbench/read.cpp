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
	if (argc < 3) {
		cerr<<"Usage: "<<argv[0]<<" <file> <read order> [rand seed]"<<endl
			<<"read order: S(sequential), D(strided), R(random), I(interleaved)"<<endl;
		return 0;
	}
	unsigned int tm;
	if (argc >= 4)
		tm = atoi(argv[3]);
	else
		tm = time(NULL);
	srand(tm);
	cerr<<"seed = "<<tm<<endl;

	const char *fileName = argv[1];
	char readOrder = argv[2][0];

	//InternalSplitter *isp = new MSplitter<PID_t>();
	//LeafSplitter *lsp = new MSplitter<Datum_t>();

	BTree *tree = new BTree(fileName);
	Key_t total = tree->upperBound();
	Key_t i,j,k,l;
	Key_t size =  (Key_t) sqrt(total);
	Key_t *keys = new Key_t[total];

	switch(readOrder) {
		// CONTINUE
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
    case 'I':
		// row 0
		for (l=0; l<size; ++l)
			keys[l] = l;
		for (k=1; k<size; ++k) {
			// column k-1 and then row k
			for (i=k; i<size; ++i)
				// (i,k-1)
				keys[l++] = i*size+k-1;
			for (i=k; i<size; ++i)
				// (k, i)
				keys[l++] = k*size+i;
		}
	}

	Datum_t datum;
	for (i=0; i<total; ++i) {
		tree->get(keys[i], datum);
	}
	delete tree;
	delete keys;
}
