#include <iostream>
#include "common/common.h"

using namespace std;

int main(int argc, char **argv)
{
	const int required = 4;
	if (argc < required) {
		cerr<<"Usage: "<<argv[0]<<" <type> <scale> <period> [seed]"<<endl
			<<"  <type>: S, D, R, I"<<endl
			<<"  <scale>: the matrix will be scale x scale"<<endl
            <<"  <period>: a nonzero every period elements, e.g. 5 means density=1/5"<<endl
			//<<"  <genVal>: if 1, value 1.0 will be written for each key"<<endl
			<<"  [seed]: random seed, optional for debugging purposes"<<endl;
		return 0;
	}
	char type = argv[1][0];
	Key_t scale = atoi(argv[2]);
    int period = atoi(argv[3]);
    //int period = 100/density; 
	int seed = time(NULL);

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
            if (i % period == 0)
                keys[k++] = i;
        if (type=='R')
            permute(keys, k);
		break;
	case 'D':
        k = 0;
        for (j=0; j<scale; ++j) 
            for (i=0; i<scale; ++i) 
                if ((i*scale+j) % period == 0)
                    keys[k++] = i*scale+j;
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
            if (k % period == 0)
                keys[l++] = k;
        }
		for (k=1; k<scale; ++k) {
			// column k-1 and then row k
			for (i=k; i<scale; ++i) {
				// (i,k-1)
                Key_t t = i*scale+k-1;
                if (t % period == 0)
                    keys[l++] = i*scale+k-1;
            }
            // row k
			for (i=k; i<scale; ++i) {
				// (k, i)
                Key_t t = k*scale+i;
                if (t % period == 0)
                    keys[l++] = t;
            }
		}
		break;
	default:
		cerr<<"type not recognized"<<endl;
		return 1;
	}

    size_t size = (total+period-1)/period;
	write(file, keys, sizeof(Key_t)*size);
	close(file);
    delete[] keys;
	return 0;
}
