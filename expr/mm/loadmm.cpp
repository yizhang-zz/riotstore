#include "common/Config.h"
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
    i64 blockDims[] = {config->matmulBlockFactor, config->matmulBlockFactor};
    u8 orders[] = {1,0};
    BlockBased<2> l_bb(blockDims, blockDims, orders, orders);
    ColMajor<2> l_col(blockDims); // bogus dim
    int batchLoadBufferSize = 4000000;
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

        // column major
        strcpy(buf, basename(path));
        strcat(buf, ".cm.lab");
        sp.type = BTREE;
        sp.btreeParam.leafSp = 'B';
        sp.btreeParam.intSp = 'M';
        MDArray<2> array1(&sp, &l_col, "MM", path, batchLoadBufferSize);

        buf[baselen] = '\0';
        strcat(buf, ".cm.b");
        sp.btreeParam.leafSp = 'M';
        MDArray<2> array2(&sp, &l_col, "MM", path, batchLoadBufferSize);

        buf[baselen] = '\0';
        strcat(buf, ".cm.dma");
        sp.type = DMA;
        MDArray<2> array3(&sp, &l_col, "MM", path, batchLoadBufferSize);

        // block based
        buf[baselen] = '\0';
        strcat(buf, ".bb.lab");
        sp.type = BTREE;
        sp.btreeParam.leafSp = 'B';
        sp.btreeParam.intSp = 'M';
        MDArray<2> array4(&sp, &l_bb, "MM", path, batchLoadBufferSize);

        buf[baselen] = '\0';
        strcat(buf, ".bb.b");
        sp.btreeParam.leafSp = 'M';
        MDArray<2> array5(&sp, &l_bb, "MM", path, batchLoadBufferSize);

        buf[baselen] = '\0';
        strcat(buf, ".bb.dma");
        sp.type = DMA;
        MDArray<2> array6(&sp, &l_bb, "MM", path, batchLoadBufferSize);
    }
}
