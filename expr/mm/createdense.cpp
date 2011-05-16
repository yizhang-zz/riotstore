#include <iostream>
#include "array/MDArray.h"
#include "array/BlockBased.h"
using namespace std;

int main(int argc, char **argv)
{
	if (argc < 3) {
		cerr<<argv[0]<<" size outfile"<<endl;
		exit(1);
	}

	StorageParam sp;
	sp.type = BTREE;
	sp.leafSp = 'A';
	sp.intSp = 'M';
	sp.useDenseLeaf = true;
	sp.fileName = argv[2];

	i64 size = atol(argv[1]);
	cout<<size<<endl;
	i64 adims[] = {atol(argv[1]), atol(argv[1])};
	i64 dims[] = {config->matmulBlockFactor, config->matmulBlockFactor};
	u8 orders[] = {1, 0};
	BlockBased<2> lin(adims, dims, orders, orders);
	MDArray<2> array(&sp, MDCoord<2>(adims), &lin);

	i64 i,j;
	MDCoord<2> coord;
	i64 nr, nc, nn=config->matmulBlockFactor;
	for (j=0; j<size; ++j)
		for (i=0; i<size; ++i) {
			coord[0] = i;
			coord[1] = j;
			array.put(coord, 1.0);
		}
}
