#include "common/Config.h"
#include "array/ColMajor.h"
#include "array/MDArray.h"
#include "btree/Btree.h"
#include <iostream>
#include <libgen.h>

using namespace std;

int main(int argc, char **argv)
{
    if (argc < 3) {
        cerr<<"usage: "<<argv[0]<<" inputlist to_types(BLD)"<<endl
            <<"inputlist: a file containing paths to MatrixMarket format files"<<endl
            <<"B: B-tree L: LAB-tree D: DirectlyMapped"<<endl;
        exit(1);
    }
    char types[256] = {0};
    for (char *p=argv[2]; *p; ++p)
        types[int(*p)] = 1;

    char *inputfile = argv[1];
    char path[256];
    ifstream in(inputfile);
    // support two linearizations: col-major and block based
    i64 blockDims[] = {config->matmulBlockFactor, config->matmulBlockFactor};
    //i64 blockDims[] = {500,500};
    u8 orders[] = {1,0};
    BlockBased<2> l_bb(blockDims, blockDims, orders, orders);
    ColMajor<2> l_col(blockDims); // bogus dim
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

        // column major
        buf[baselen] = '\0';
        strcat(buf, ".cm.lab");
        sp.type = BTREE;
        sp.leafSp = 'A';
        sp.intSp = 'M';
        sp.useDenseLeaf = true;
        if (types['L']) {
            MDArray<2> array1(&sp, &l_col, "MM", path, batchLoadBufferSize);
            //((Btree::BTree*)array1.storage)->print(true);
        }

        buf[baselen] = '\0';
        strcat(buf, ".cm.b");
        sp.leafSp = 'M';
        sp.useDenseLeaf = false;
        if (types['B']) {
            MDArray<2> array2(&sp, &l_col, "MM", path, batchLoadBufferSize);
            //((Btree::BTree*)array2.storage)->print(true);
        }
        buf[baselen] = '\0';
        strcat(buf, ".cm.dma");
        sp.type = DMA;
        if (types['D'])
            MDArray<2> array3(&sp, &l_col, "MM", path, batchLoadBufferSize);

		/*
        // block based
        buf[baselen] = '\0';
        strcat(buf, ".bb.lab");
        sp.type = BTREE;
        sp.leafSp = 'A';
        sp.intSp = 'M';
        sp.useDenseLeaf = true;
        if (types['L']) {
            MDArray<2> array4(&sp, &l_bb, "MM", path, batchLoadBufferSize);
            //((Btree::BTree*)array4.storage)->print(true);
        }

        buf[baselen] = '\0';
        strcat(buf, ".bb.b");
        sp.leafSp = 'M';
        sp.useDenseLeaf = false;
        if (types['B']) {
            MDArray<2> array5(&sp, &l_bb, "MM", path, batchLoadBufferSize);
            //((Btree::BTree*)array5.storage)->print(true);
        }

        buf[baselen] = '\0';
        strcat(buf, ".bb.dma");
        sp.type = DMA;
        if (types['D'])
            MDArray<2> array6(&sp, &l_bb, "MM", path, batchLoadBufferSize);
			*/
    }
}
