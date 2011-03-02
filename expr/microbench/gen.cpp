#include <iostream>
#include "common/common.h"

using namespace std;

int main(int argc, char **argv)
{
	const int required = 4;
	if (argc < required) {
		cerr<<"Usage: "<<argv[0]<<" <type> <scale> <nonzero prob.> [seed]"<<endl
			<<"  <type>: S, D, R, I"<<endl
			<<"  <scale>: the matrix will be scale x scale"<<endl
            <<"  <nonzero prob.>: the probability of an every being nonzero"<<endl
			//<<"  <genVal>: if 1, value 1.0 will be written for each key"<<endl
			<<"  [seed]: random seed, optional for debugging purposes"<<endl;
		return 0;
	}
	char type = argv[1][0];
	Key_t scale = atoi(argv[2]);
	int seed = time(NULL);
    if (argc >= 5)
        seed = atoi(argv[4]);
    srand(seed);
    double prob = atof(argv[3]);
    int threshold = (int) (prob * (RAND_MAX+1.0)-1);
    cerr<<"RAND_MAX="<<RAND_MAX<<endl;
    cerr<<"threshold="<<threshold<<endl;

	char filename[100];
	sprintf(filename, "%c%d", type, (int)scale);
	int file = open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0660);

	Key_t total = scale*scale;
	Key_t *keys = new Key_t[total];
	Key_t i,j,k,l;
	switch (type) {
	case 'S':
    case 'R':
        k = 0;
		for (i=0; i<total; ++i) 
            if (rand() <= threshold)
                keys[k++] = i;
        if (type=='R')
            permute(keys, k);
		break;
	case 'D':
        k = 0;
        for (j=0; j<scale; ++j) 
            for (i=0; i<scale; ++i) 
                if (rand() <= threshold)
                    keys[k++] = i*scale+j;
		break;
	case 'I':
		// interleaved pattern:
		// ------
		// |-----
		// ||----
		// |||---

		// row 0
        k = 0;
		for (l=0; l<scale; ++l) {
            if (rand() <= threshold)
                keys[k++] = l;
        }
		for (l=1; l<scale; ++l) {
			// column l-1 and then row l
			for (i=l; i<scale; ++i) {
				// (i,l-1)
                if (rand() <= threshold)
                    keys[k++] = i*scale+l-1;
            }
            // row l
			for (i=l; i<scale; ++i) {
				// (l, i)
                if (rand() <= threshold)
                    keys[k++] = l*scale+i;
            }
		}
		break;
	default:
		cerr<<"type not recognized"<<endl;
		return 1;
	}

    Key_t written = 0;
    while (written < k) {
        ssize_t w = write(file, keys+written, sizeof(Key_t)*(k-written)) / sizeof(Key_t);
        cerr<<"written "<<w<<" elements"<<endl;
        written += w;
    }
	close(file);
    delete[] keys;
	return 0;
}
