#include "array/MDArray.h"
#include "array/ColMajor.h"
#include "array/BlockBased.h"
#include <iostream>
#include <libgen.h>

using namespace std;

int main(int argc, char **argv)
{
    if (argc < 2) {
        cerr<<"usage: "<<argv[0]<<" inputlist.txt"<<endl
            <<"where inputlist.txt contains paths to MatrixMarket format files"<<endl;
        exit(1);
    }
    char *inputfile = argv[1];
    char path[256];
    ifstream in(inputfile);
    // support two linearizations: col-major and block based
    i64 blockDims[] = {5000,5000};
    u8 orders[] = {1,0};
    BlockBased<2> l_bb(blockDims, blockDims, orders, orders);
    ColMajor<2> l_col(blockDims); // bogus dim
    while (in.good()) {
        in.getline(path, 256);
        if (!in.good())
            break;
        if (path[0]=='#')
            continue;
        cout<<"reading file "<<path<<endl;
        char buf[256];
        strcpy(buf, basename(path));
        strcat(buf, ".cm.lab");
        MDArray<2> array(buf, &l_col, 'B', 'M', "MM", path, 1000000);
    }
}
