#include <iostream>
#include "array/MDArray.h"
#include "array/BlockBased.h"
using namespace std;

int main(int argc, char **argv)
{
	if (argc < 6) {
		cerr<<argv[0]<<" size1 size2 bf1 bf2 outfile"<<endl;
		exit(1);
	}

	StorageParam sp;
	sp.type = BTREE;
	sp.leafSp = 'A';
	sp.intSp = 'M';
	sp.useDenseLeaf = true;
	sp.fileName = argv[5];

	i64 size1 = atol(argv[1]);
	i64 size2 = atol(argv[2]);
	i64 bf1 = atol(argv[3]);
	i64 bf2 = atol(argv[4]);
	i64 adims[] = {size1, size2};
	i64 dims[] = {bf1, bf2};
	u8 orders[] = {1, 0};
	BlockBased<2> lin(adims, dims, orders, orders);
	MDArray<2> array(&sp, MDCoord<2>(adims), &lin);

	i64 i,j;
	MDCoord<2> coord;
	i64 nr, nc, nn=config->matmulBlockFactor;
	for (i=0; i<size1; ++i)
		for (j=0; j<size2; ++j)
		{
			coord[0] = i;
			coord[1] = j;
			array.put(coord, 1.0);
		}
}
