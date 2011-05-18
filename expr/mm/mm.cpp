#include "array/MDArray.h"
#include "array/ColMajor.h"
#include "array/BlockBased.h"
#include <iostream>
using namespace std;

int main(int argc, char **argv)
{
    if (argc < 6) {
        cerr<<"usage: "<<argv[0]<<" input1 input2 blocking_factor1 bf2 bf3"<<endl;
        exit(1);
    }

    Matrix a(argv[1]);
    Matrix b(argv[2]);
    i64 p = atol(argv[3]);
    i64 q = atol(argv[4]);
    i64 r = atol(argv[5]);
    Matrix c(a.multiply(b, p, q, r));
}
