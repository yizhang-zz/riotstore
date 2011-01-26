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

    BTree tree1(argv[1]);
    BTree tree2(argv[2]);

    if (tree1.upperBound() != tree2.upperBound()
            || tree1.nnz() != tree1.nnz())
    {
        cerr << "range or nnz nonequal"<<endl;
        return 1;
    }

    Key_t end = tree1.upperBound();
    double x,y;
    for (Key_t k = 0; k < end; ++k) {
        tree1.get(k,x);
        tree2.get(k,y);
        if (x != y) {
            cerr << "key="<<k<<", "<<x<<"!="<<y<<endl;
            return 1;
        }
    }
    return 0;
}
