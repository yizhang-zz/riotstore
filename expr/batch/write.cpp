#include <stdio.h>
#include <libgen.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include "common.h"
#include "Config.h"
#include "Btree.h"

using namespace std;
using namespace Btree;

const int batchSize = 10000000;
Key_t keys[batchSize];

int main(int argc, char **argv)
{
	const int required = 4;
	if (argc < required) {
		cerr<<"Usage: "<<argv[0]<<" <input file> <splitter type> <dest dir>"<<endl
			<<"input file: e.g., S2000 (sequential sequence of 2k x 2k matrix"<<endl
			<<"splitter type: M,B,R,S,T"<<endl
			<<"dest dir: where to write the result file (named mb)"<<endl;
		return 0;
	}

	char fileName[100];
    strcpy(fileName, argv[3]);
    strcat(fileName, "/mb");
	char *infilename = argv[1];
	char splitterType = argv[2][0];

	int infile = open(infilename, O_RDONLY);

	int size = atoi(basename(infilename)+1);
	Key_t total = Key_t(size) * size;
    BTree *tree = new BTree(fileName, total, splitterType, 'M', config->useDenseLeaf);

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
}
