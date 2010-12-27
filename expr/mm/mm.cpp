#include "array/MDArray.h"
#include "array/ColMajor.h"
#include "array/BlockBased.h"
#include <iostream>
using namespace std;

int main(int argc, char **argv)
{
    if (argc < 2) {
        cerr<<"usage: "<<argv[0]<<" input"<<endl;
        exit(1);
    }

    Matrix a(argv[1]);
    Matrix b(argv[1]);
    Matrix c(a*b);
}
