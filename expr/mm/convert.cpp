#include "common/Config.h"
#include "array/ColMajor.h"
#include "array/MDArray.h"
#include "btree/Btree.h"
#include <iostream>
#include <libgen.h>

using namespace std;

int main(int argc, char **argv)
{
	if (argc < 5) {
		cerr<<"usage: "<<argv[0]<<" input output target_bf1 target_bf2"<<endl;
		exit(1);
	}

	char *inputfile = argv[1];
	MDArray<2> in_array(inputfile);
	MDCoord<2> arrayDims = in_array.getDims();
	// output linearization
	i64 blockDims[] = {atol(argv[3]), atol(argv[4])};
	u8 orders[] = {0,1};
	BlockBased<2> lin(&arrayDims[0], blockDims, orders, orders);
	StorageParam sp;
	sp.fileName = argv[2];
	sp.type = BTREE;
	sp.leafSp = 'A';
	sp.intSp = 'M';
	sp.useDenseLeaf = true;
	MDArray<2> out_array(&sp, arrayDims, &lin);
	// read blocks of out_array's shape from in_array
	int nr = (arrayDims[0]+blockDims[0]-1)/blockDims[0];
	int nc = (arrayDims[1]+blockDims[1]-1)/blockDims[1];
	double *buf = (double*) malloc(sizeof(double)*blockDims[0]*blockDims[1]);
	for (int i=0; i<nr; ++i) {
		for (int j=0; j<nc; ++j) {
			MDCoord<2> begin(i*blockDims[0], j*blockDims[1]);
			MDCoord<2> end((i+1)*blockDims[0]-1, (j+1)*blockDims[1]-1);
			in_array.batchGet(begin, end, buf);
			out_array.batchPut(begin, end, buf);
			//std::vector<MDArrayElement<2> > v;
			//in_array.batchGet(begin, end, v);
			//out_array.batchPut(v);
		}
	}
	free(buf);
}
