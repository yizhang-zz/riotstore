#include <iostream>
#include "common/common.h"

using namespace std;

int main(int argc, char **argv)
{
	const int required = 3;
	if (argc < required) {
		cerr<<"Usage: "<<argv[0]<<" <type> <scale> "<<endl
			<<"  <type>: S, D, R "<<endl
			<<"  <scale>: the matrix will be scale x scale"<<endl;
		return 0;
	}
	char type = argv[1][0];
	Key_t scale = atoi(argv[2]);

	char filename[100];
	sprintf(filename, "%c%d", type, (int)scale);
	int file = open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0660);

	Key_t total = scale*(scale+1)/2;
	Key_t *keys = new Key_t[total];
	Key_t i,j,k=0;
	switch (type) {
	case 'S':
	case 'R':
		for (i=0; i<scale; ++i) 
            for (j=0; j<=i; ++j)
                keys[k++] = i*scale+j;
        if (type=='R')
            permute(keys, total);
		break;
	case 'D':
		for (j=0; j<scale; ++j)
            for (i=j; i<scale; ++i)
                keys[k++] = i*scale+j;
		break;
        // interleaved is the same as strided.
	default:
		cerr<<"type not recognized"<<endl;
		return 1;
	}

	write(file, keys, sizeof(Key_t)*total);
	close(file);
    delete[] keys;
	return 0;
}
