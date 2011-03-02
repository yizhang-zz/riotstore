#include <stdio.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include "common.h"
#include "Config.h"
#include "Btree.h"

using namespace std;
using namespace Btree;

int main(int argc, char **argv)
{
	if (argc < 3) {
		cerr<<"Usage: "<<argv[0]<<" <input1> <input2>"<<endl;
		return 0;
	}

    LinearStorage* ls1 = LinearStorage::fromFile(argv[1]);
    LinearStorage* ls2 = LinearStorage::fromFile(argv[2]);

    if (ls1->upperBound() != ls2->upperBound()
            || ls1->nnz() != ls2->nnz())
    {
        cerr << "range or nnz nonequal"<<endl;
        return 1;
    }

    Key_t end = ls1->upperBound();
    double x,y;
    for (Key_t k = 0; k < end; ++k) {
        ls1->get(k,x);
        ls2->get(k,y);
        if (x != y) {
            cerr << "key="<<k<<", "<<x<<"!="<<y<<endl;
            return 1;
        }
    }
    return 0;
}
