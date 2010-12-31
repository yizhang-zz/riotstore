#include "array/MDArray.h"
#include "array/ColMajor.h"
#include "array/BlockBased.h"
#include <iostream>
using namespace std;

int main(int argc, char **argv)
{
    if (argc < 3) {
        cerr<<"usage: "<<argv[0]<<" input"<<endl;
        exit(1);
    }

    Matrix a(argv[1]);
    Matrix b(argv[2]);
    int start, stop;
    //start = atoi(argv[3]);
    //stop = atoi(argv[4]);
    for (Key_t j = 0; j<a.getDims()[1]; ++j) {
        for (Key_t i = 0; i<a.getDims()[0]; ++i) {
			//Key_t total = a.getDims()[0]*a.getDims()[1];
			//for (Key_t i = 0; i<total; i++) {
			Datum_t d1, d2;
			MDCoord<2> coord(i,j);
			a.get(coord, d1);
			b.get(coord, d2);
			assert(fabs(d1-d2)<1e-6);
        }
        cout<<"col "<<j<<" done"<<endl;
        }
}
