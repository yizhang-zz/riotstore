#include <iostream>
#include "common/common.h"

using namespace std;

int main(int argc, char **argv)
{
	const int required = 4;
	if (argc < required) {
		cerr<<"Usage: "<<argv[0]<<" <type> <scale> <density> [seed]"<<endl
			<<"  <type>: S, D, R, I"<<endl
			<<"  <scale>: the matrix will be scale x scale"<<endl
            <<"  <density>: %% of nonzeros, e.g. 50 means 50/100"<<endl
			//<<"  <genVal>: if 1, value 1.0 will be written for each key"<<endl
			<<"  [seed]: random seed, optional for debugging purposes"<<endl;
		return 0;
	}
	char type = argv[1][0];
	Key_t scale = atoi(argv[2]);
    int density = atoi(argv[3]);
    int period = 100/density; 
	int seed = time(NULL);

	char filename[100];
	sprintf(filename, "%c%d", type, (int)scale);
	int file = open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0660);

	Key_t total = scale*scale;
	Key_t *keys = new Key_t[total];
	Key_t i,k,l;
	switch (type) {
	case 'S':
		for (i=0; i<total; ++i) 
            if (i % period == 0)
                keys[i/period] = i;
		break;
	case 'D':
		for (i=0; i<total; ++i) 
            if (i % period == 0)
                keys[i/period] = period * (i % period) + i / period;
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
        l = 0;
		for (k=0; k<scale; ++k) {
            if (l % period == 0)
                keys[l/period] = k;
            ++l;
        }
		for (k=1; k<scale; ++k) {
			// column k-1 and then row k
			for (i=k; i<scale; ++i) {
				// (i,k-1)
                if (l % period == 0)
                    keys[l/period] = i*scale+k-1;
                ++l;
            }
			for (i=k; i<scale; ++i) {
				// (k, i)
                if (l % period == 0)
                    keys[l/period] = k*scale+i;
                ++l;
            }
		}
        assert(l==total);
		break;
	default:
		cerr<<"type not recognized"<<endl;
		return 1;
	}

	write(file, keys, sizeof(Key_t)*total*density/100);
	close(file);
    delete[] keys;
	return 0;
}
