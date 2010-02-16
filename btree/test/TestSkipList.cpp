#include "../SkipList.h"
#include <gtest/gtest.h>
#include <iostream>
#include "../../common/common.h"
using namespace std;

TEST(SkipList, All)
{
    //apr_pool_t *pool;
    //apr_pool_create(&pool, NULL);
    srand(time(NULL));
    //srand(113457);
    SkipList<int, int> sl(NULL);
    char buf[10];
    const int num = 20;
    int data[num];
    for (int i=0; i<num; i++)
        data[i] = i;
    permute<int>(data, num);

    for (int i=0; i<1; i++) {
        int value;
        int index;
        ASSERT_EQ(sl.NotFound, sl.search(i, value, index));
        ASSERT_EQ(index, 0);
    }
    
    for (int i=0; i<num; i++) {
        ASSERT_EQ(sl.OK, sl.insert(data[i],data[i]));
        //sprintf(buf, "%d.dot", i);
        //sl.print();
    }

    for (int i=0; i<num; i++) {
        int value;
        int index;
        ASSERT_EQ(sl.OK, sl.search(i,value,index))<<i;
        ASSERT_EQ(value, i);
    }

    for (int i=0; i<1; i++) {
        int value;
        int index;
        ASSERT_EQ(sl.NotFound, sl.search(20, value, index));
        ASSERT_EQ(index, 20);
    }
 
    // sl.print();
    for (int i=0; i<num; i++) {
        int key;
        int value;
        ASSERT_EQ(sl.OK, sl.locate(i, key, value));
        ASSERT_EQ(key, i);
        ASSERT_EQ(value, i);
    }

    // iterator
    SkipList<int,int>::Iterator *it = sl.getIterator();
    int i = 0;
    while (it->moveNext()) {
        int key, value;
        it->get(key, value);
        ASSERT_EQ(key, i);
        ASSERT_EQ(value, i);
        i++;
    }
    it->setIndexRange(num/5, num/2);
    i = num/5;
    while (it->moveNext()) {
        int key, value;
        it->get(key, value);
        ASSERT_EQ(key, i);
        ASSERT_EQ(value, i);
        i++;
    }
    delete it;

    // remove random elements
    int rcount = num/3;
    int remain = num;
    for (int i=0; i<rcount; i++) {
        int key = rand()%num;
        int value;
        int index;
        if (sl.OK == sl.remove(key)) {
            //cout<<"::removing "<<key<<endl;
            //sl.print();
            remain--;
            ASSERT_EQ(sl.getSize(), remain);
            ASSERT_EQ(sl.NotFound, sl.search(key, value, index));
        }
        else {
            ASSERT_EQ(sl.getSize(), remain);
        }
    }

    // removeAt
    cout<<"before removeAt(0)"<<endl;
    sl.print();
    cout<<"after removeAt(0)"<<endl;
    sl.removeAt(0);
    sl.print();
    cout<<"after removeAt(num/3)"<<endl;
    sl.removeAt(num/3);
    sl.print();
    

    //sl.print();

    // truncate
    //cout<<"****** after truncation ******"<<endl;
    int left = sl.getSize()/2;
    sl.truncate(left);
    ASSERT_EQ(sl.getSize(), left);
    //sl.print();
    //apr_pool_destroy(pool);
    //apr_terminate();
}

int main(int argc, char **argv) {
    srand(103948);
    apr_initialize();
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

