#include <gtest/gtest.h>
#include "../BtreeBlock.h"
#include "../Btree.h"
#include "../Splitter.h"

TEST(Btree, Create)
{
    MSplitter sp;
    Btree tree("tree1.dat", 100, &sp, &sp);
    tree.print();
    Key_t key = 0;
    Datum_t datum = key;
    for (int i=0; i<26; i++) {
        key = i;
        datum = i;
        tree.put(key, datum);
    }
    tree.print();

}
