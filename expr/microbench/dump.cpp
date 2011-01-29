#include <iostream>
#include "common/common.h"
#include "common/Config.h"
#include "btree/Btree.h"

using namespace std;
using namespace Btree;

int main(int argc, char **argv)
{
    cout<<config->sparseLeafCapacity<<endl;
    cout<<config->denseLeafCapacity<<endl;
    cout<<config->internalCapacity<<endl;
    BTree bt(argv[1]);
    bt.print(LSP_FULL);
}
