#include <libgen.h>
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
	if (argc < 3) {
		cerr<<"Usage: "<<argv[0]<<" <file> <read order> [rand seed]"<<endl
			<<"read order: S(sequential), D(strided), R(random row), I(interleaved)"<<endl;
		return 0;
	}
	unsigned int tm;
	if (argc >= 4)
		tm = atoi(argv[3]);
	else
		tm = time(NULL);
	srand(tm);
	cerr<<"seed = "<<tm<<endl;

	char *fileName = argv[1];
	char readOrder = argv[2][0];

    // fileName = [insert order][scale][splitter type]-...
    // splitter type of 'D' means DMA, others B-tree
    char *p = basename(fileName);
    char seq, storage;
    Key_t rows, cols;
    char srcfile[100] = {0};
    if (strstr(fileName, "x")) {
        sscanf(p, "%c%lux%lu%c", &seq, &rows, &cols, &storage);
        sprintf(srcfile, "%c%lux%lu", seq, rows, cols);
    }
    else {
        sscanf(p, "%c%lu%c", &seq, &rows, &storage);
        sprintf(srcfile, "%c%lu", seq, rows);
        cols = rows;
    }
    cerr<<"src="<<srcfile<<endl;

    LinearStorage *ls = NULL;
    if (storage == 'D')
        ls = new DirectlyMappedArray(fileName, 0);
    else
        ls = new BTree(fileName);
	Key_t total = ls->upperBound();
	Key_t i,j,k,l;
	Key_t size =  (Key_t) sqrt(total);
	Key_t *keys = new Key_t[total];
    int *rowIndices = new int[rows];
	Datum_t datum;

	switch(readOrder) {
	case 'S':
		for (i=0; i<total; ++i) {
            ls->get(i, datum);
		}
		break;
	case 'D':
		for (i=0; i<size; ++i) 
			for (j=0; j<size; ++j) 
				keys[i*size+j] = j*size+i;
		break;
	case 'R':
        /*
        {
	const int batchSize = 1000000;
    int infile = open(srcfile, O_RDONLY);
    Key_t readcount=0;
	while (true) {
		ssize_t c = read(infile, keys+readcount, batchSize*sizeof(Key_t));
		int count = c/sizeof(Key_t);
        readcount += count;
		if (count < batchSize)
			break;
	}
	close(infile);
    cerr<<"read count="<<readcount<<endl;
    permute(keys, readcount);
    for (i=0; i<readcount; ++i)
        ls->get(keys[i], datum);
        }
        */

		for (i=0; i<rows; ++i) 
			rowIndices[i] = i;
		permute(rowIndices, rows); // permute row indices
		for (i=0; i<rows; ++i) 
			for (j=0; j<cols; ++j) {
                ls->get(rowIndices[i]*cols+j, datum);
				//keys[i*size+j] = rowIndices[i]*size+j;
            }
		break;
        /*
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
        */
	}

	delete ls;
	delete keys;
    delete rowIndices;
}
