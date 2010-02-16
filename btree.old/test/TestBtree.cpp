#include <gtest/gtest.h>
#include "../BtreeBlock.h"
#include "../BtreeDLeafBlock.h"
#include "../Btree.h"
#include "../Splitter.h"
#include <iostream>
#include <fstream>

using namespace std;

TEST(Btree, Create)
{
    ofstream out("m.txt");
    
    BSplitter bsp(BtreeDLeafBlock::capacity);
    MSplitter msp;
    Key_t dim = 10000;
    Btree tree("tree1.dat", dim, &msp, &msp);
    // tree.print();
    Key_t key = 0;
    Datum_t datum = key;
    for (int i=0; i<dim; i++) {
        key = i;
        datum = 1.0;
        tree.put(key, datum);
        out<<i<<"\t"<<tree.getNumLeaves()<<"\n";
    }
    cout<<"# leaves "<<tree.getNumLeaves()<<endl;

}
