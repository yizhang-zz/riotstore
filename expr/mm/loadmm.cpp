#include "common/Config.h"
#include "array/ColMajor.h"
#include "array/RowMajor.h"
#include "array/MDArray.h"
#include "btree/Btree.h"
#include <iostream>
#include <libgen.h>

using namespace std;

int main(int argc, char **argv)
{
    if (argc < 4) {
        cerr<<"usage: "<<argv[0]
            <<" inputlist linearization(CRB) to_types(BLD)"<<endl
            <<"inputlist: a file containing paths to MatrixMarket format files"<<endl
            <<"linearization: Column, Row, or Block"<<endl
            <<"B: B-tree L: LAB-tree D: DirectlyMapped"<<endl;
        exit(1);
    }
    char types[256] = {0};
    for (char *p=argv[3]; *p; ++p)
        types[int(*p)] = 1;

    char *inputfile = argv[1];
    char path[256];
    ifstream in(inputfile);
    // support two linearizations: col-major and block based
    i64 blockDims[] = {config->matmulBlockFactor, config->matmulBlockFactor};
    u8 orders[] = {1,0};
    Linearization<2> *lin = NULL;
    const char *linstr;
    // we use bogus array dims because the batch load ctor of MDArray
    // will detect and apply the actual dim
    switch (argv[2][0]) {
    case 'B':
        lin = new BlockBased<2>(blockDims, blockDims, orders, orders);
        linstr = "bb";
        break;
    case 'C':
        lin = new ColMajor<2> (blockDims);
        linstr = "cm";
        break;
    case 'R':
        lin = new RowMajor<2> (blockDims);
        linstr = "rm";
        break;
    default:
        cerr<<"unknown linearization"<<endl;
    }

    int batchLoadBufferSize = blockDims[0] * blockDims[1];
    while (in.good()) {
        in.getline(path, 256);
        if (!in.good())
            break;
        if (path[0]=='#')
            continue;
        cout<<"reading file "<<path<<endl;
        char buf[256];
        StorageParam sp;
        sp.fileName = buf;
        int baselen = strlen(basename(path));
        strcpy(buf, basename(path));
        // now the linearization part
        buf[baselen++] = '.';
        buf[baselen] = '\0';
        strcat(buf, linstr);
        baselen += strlen(linstr);

        buf[baselen] = '\0';
        strcat(buf, ".lab");
        sp.type = BTREE;
        sp.leafSp = 'A';
        sp.intSp = 'M';
        sp.useDenseLeaf = true;
        if (types['L']) {
            MDArray<2> array1(&sp, lin, "MM", path, batchLoadBufferSize);
            //((Btree::BTree*)array1.storage)->print(true);
        }

        buf[baselen] = '\0';
        strcat(buf, ".b");
        sp.leafSp = 'M';
        sp.useDenseLeaf = false;
        if (types['B']) {
            MDArray<2> array2(&sp, lin, "MM", path, batchLoadBufferSize);
            //((Btree::BTree*)array2.storage)->print(true);
        }

        buf[baselen] = '\0';
        strcat(buf, ".dma");
        sp.type = DMA;
        if (types['D'])
            MDArray<2> array3(&sp, lin, "MM", path, batchLoadBufferSize);
    }
}
