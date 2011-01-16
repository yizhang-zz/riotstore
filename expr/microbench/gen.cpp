#include <iostream>
#include "common/common.h"

using namespace std;

int main(int argc, char **argv)
{
	const int required = 3;
	if (argc < required) {
		cerr<<"Usage: "<<argv[0]<<" <type> <scale> [seed]"<<endl
			<<"  <type>: S, D, R, I"<<endl
			<<"  <scale>: the matrix will be scale x scale"<<endl
			//<<"  <genVal>: if 1, value 1.0 will be written for each key"<<endl
			<<"  [seed]: random seed, optional for debugging purposes"<<endl;
		return 0;
	}
	char type = argv[1][0];
	int scale = atoi(argv[2]);
	//char genVal = argv[3][0];
	int seed = time(NULL);

	char filename[100];
	sprintf(filename, "%c%d", type, scale);
	int file = open(filename, O_WRONLY|O_CREAT, 0660);

	Key_t total = int(scale)*scale;
	Key_t *keys = new Key_t[total];
	Key_t i,j,k,l;
	switch (type) {
	case 'S':
		for (i=0; i<total; ++i) 
			keys[i] = i;
		break;
	case 'D':
		for (i=0; i<scale; ++i)
			for (j=0; j<scale; ++j) {
				keys[i*scale+j] = j*scale+i;
			}
		break;
	case 'R':
		if (argc >= required+1) 
			seed = atoi(argv[required]);
		srand(seed);
		cerr<<"seed="<<seed<<endl;
		for (i=0; i<total; ++i) 
			keys[i] = i;
		permute(keys, total);
		break;
	case 'I':
		// interleaved pattern:
		// ------
		// |-----
		// ||----
		// |||---

		// row 0
		for (l=0; l<scale; ++l)
			keys[l] = l;
		for (k=1; k<scale; ++k) {
			// column k-1 and then row k
			for (i=k; i<scale; ++i)
				// (i,k-1)
				keys[l++] = i*scale+k-1;
			for (i=k; i<scale; ++i)
				// (k, i)
				keys[l++] = k*scale+i;
		}
		break;
	default:
		cerr<<"type not recognized"<<endl;
		return 1;
	}

	write(file, keys, sizeof(Key_t)*total);
	close(file);
	return 0;
}
