#include <iostream>
#include "common/common.h"
#include "common/Config.h"

using namespace std;

int main(int argc, char **argv)
{
    cout<<config->denseLeafCapacity<<endl;
    cout<<config->sparseLeafCapacity<<endl;
}
