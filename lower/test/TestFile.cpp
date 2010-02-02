#include <gtest/gtest.h>
#include <stdlib.h>
#include <string.h>
#include "../../common/common.h"
#include "../BitmapPagedFile.h"
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
using namespace std;

/*
PID_t pid;
Byte_t filler[PAGE_SIZE];
FILE* f;
PageHandle ph;*/

TEST(BmpPagedFile, constructor) 
{
    // create new file
    BitmapPagedFile *bpf = new BitmapPagedFile("test1.bin", BitmapPagedFile::F_CREATE);
RC_t rc;
    ASSERT_EQ(rc , RC_SUCCESS);
    for(int k=0; k<PAGE_SIZE; k++) {
        ASSERT_FALSE(bpf->isAllocated(k));
    }
    ASSERT_EQ(bpf->numContentPages , 0);

    bpf->allocate(0);
    ASSERT_EQ(bpf->header[0] , 1);
    ASSERT_TRUE(bpf->isAllocated(0));
    delete bpf;

    // open same file
    bpf = new BitmapPagedFile("test1.bin", BitmapPagedFile::F_NO_CREATE);
    ASSERT_TRUE(bpf);
    ASSERT_TRUE(bpf->isAllocated(0));
    ASSERT_EQ(bpf->header[0] , 1);
    ASSERT_EQ(bpf->numContentPages , 0);
    for(int k=1; k<PAGE_SIZE; k++) {
        ASSERT_FALSE(bpf->isAllocated(k));
    }
    delete bpf;

    // open some other file
    bpf = new BitmapPagedFile("test2.bin", BitmapPagedFile::F_CREATE);
    ASSERT_TRUE(bpf);
    for(int k=0; k<PAGE_SIZE; k++) {
        ASSERT_TRUE(!bpf->isAllocated(k));
    }
    delete bpf;

	remove("test1.bin");
}

TEST(BmpPagedFile, allocatePage)
{
PID_t pid;
    BitmapPagedFile *bpf = new BitmapPagedFile("test1.bin", BitmapPagedFile::F_CREATE);
    ASSERT_TRUE(bpf);

    // allocate each bit sequentially in header
    for(int k=0; k<8*PAGE_SIZE; k++) {
        ASSERT_EQ(bpf->allocatePage(pid) , RC_SUCCESS);
        ASSERT_EQ(pid , k);
        ASSERT_EQ(bpf->numContentPages , k+1);
    }

    // allocate past range
    ASSERT_EQ(bpf->allocatePage(pid) , RC_FAILURE);

    // allocate holes from least to greatest
    bpf->header[3] = 0;
    bpf->header[123] = 0;
    bpf->header[4000] = 0;

    for(int k=0; k<8; k++) {
        ASSERT_EQ(bpf->allocatePage(pid) , RC_SUCCESS);
        ASSERT_EQ(pid , 3*8+k);
    }
    for(int k=0; k<8; k++) {
        ASSERT_EQ(bpf->allocatePage(pid) , RC_SUCCESS);
        ASSERT_EQ(pid , 123*8+k);
    }
    for(int k=0; k<8; k++) {
        ASSERT_EQ(bpf->allocatePage(pid) , RC_SUCCESS);
        ASSERT_EQ(pid , 4000*8+k);
    }
    ASSERT_EQ(bpf->allocatePage(pid) , RC_FAILURE);
    delete bpf;
remove("test1.bin");
	remove("test2.bin");

}

TEST(BmpPagedFile, allocatedPagePID)
{

    BitmapPagedFile *bpf = new BitmapPagedFile("test1.bin", BitmapPagedFile::F_CREATE);
    ASSERT_TRUE(bpf);

    // allocate each PID once
    for(int k=0; k<8*PAGE_SIZE; k++) {
        ASSERT_EQ(bpf->allocatePageWithPID(k) , RC_SUCCESS);
        ASSERT_EQ(bpf->numContentPages , k+1);
        ASSERT_EQ(bpf->allocatePageWithPID(k) , RC_FAILURE);
        ASSERT_EQ(bpf->numContentPages , k+1);
    }

    ASSERT_EQ(bpf->allocatePageWithPID(8*PAGE_SIZE) , RC_FAILURE);
    delete bpf;

    // header and contentPages written and retrieved
    bpf = new BitmapPagedFile("test1.bin", BitmapPagedFile::F_NO_CREATE);
    ASSERT_TRUE(bpf);
    //fseek(bpf->file, 0, SEEK_END);
    ASSERT_EQ(bpf->numContentPages , 8*PAGE_SIZE);
	struct stat s;
	fstat(bpf->fd, &s);
    ASSERT_EQ(s.st_size , (8*PAGE_SIZE+1)*PAGE_SIZE);
    for(int k=0; k<PAGE_SIZE; k++) {
        ASSERT_EQ(bpf->header[k] , 255);
    }
    delete bpf;

    bpf = new BitmapPagedFile("test2.bin", BitmapPagedFile::F_CREATE);
    ASSERT_TRUE(bpf);
    ASSERT_EQ(bpf->allocatePageWithPID(202) , RC_SUCCESS);
    delete bpf;

    bpf = new BitmapPagedFile("test2.bin", BitmapPagedFile::F_NO_CREATE);
    ASSERT_TRUE(bpf);
    ASSERT_EQ(bpf->numContentPages , 203);
	fstat(bpf->fd, &s);
    ASSERT_EQ(s.st_size , (204)*PAGE_SIZE);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==25) {
            ASSERT_EQ(bpf->header[k] , 4);
        }
        else
            ASSERT_EQ(bpf->header[k] , 0);
    }
    delete bpf;
remove("test1.bin");
	remove("test2.bin");

}

TEST(BmpPagedFile, disposePage)
{

    BitmapPagedFile *bpf = new BitmapPagedFile("test1.bin", BitmapPagedFile::F_CREATE);
    ASSERT_TRUE(bpf);

    bpf->header[4] = 130; // 10000010
    bpf->header[1543] = 47; // 00101111
    bpf->numContentPages = 15000;

    ASSERT_EQ(bpf->disposePage(39) , RC_SUCCESS);
    ASSERT_EQ(bpf->header[4] , 2);
    ASSERT_EQ(bpf->disposePage(12345) , RC_SUCCESS);
    ASSERT_EQ(bpf->header[1543] , 45);

    for(int k=0; k<8*PAGE_SIZE; k++) {
        if(k==33||k==12344||k==12346||k==12347||k==12349) {
            ASSERT_EQ(bpf->disposePage(k) , RC_SUCCESS);
        }
        else {
            ASSERT_EQ(bpf->disposePage(k) , RC_FAILURE);
        }
    }

    // ASSERT_EQ(bpf->disposePage(-1) , RC_FAILURE);
    ASSERT_EQ(bpf->disposePage(8*PAGE_SIZE) , RC_FAILURE);

    delete bpf;
remove("test1.bin");
}

TEST(BmpPagedFile, readWrite)
{
PageHandle ph;
Byte_t filler[PAGE_SIZE];
    BitmapPagedFile *bpf = new BitmapPagedFile("test1.bin", BitmapPagedFile::F_CREATE);
    ASSERT_TRUE(bpf);

    for(int k=0; k<PAGE_SIZE; k++) {
        memset(filler+k, k%256, 1);
    }
    ph.image = &filler;
    ph.pid = 20;

    ASSERT_EQ(bpf->writePage(ph) , RC_FAILURE); // pid out of range and
                                                // unallocated

    bpf->header[2] = 16; // 00010000
    bpf->numContentPages = 20;
    ASSERT_EQ(bpf->writePage(ph) , RC_FAILURE); // pid out of range

    // pad contentPages on file
    bpf->numContentPages = 30;
    ASSERT_EQ(bpf->writePage(ph) , RC_SUCCESS);
    // fflush(bpf->file);
	struct stat s;
	fstat(bpf->fd, &s);
	ASSERT_EQ(s.st_size , 22*PAGE_SIZE);

    // write to one of contentPages
    bpf->header[1] = 1; // 00000001
    ph.pid = 8;
    ASSERT_EQ(bpf->writePage(ph) , RC_SUCCESS);

    // expand contentPages
    bpf->header[3] = 32; // 00100000
    ph.pid = 29;
    ASSERT_EQ(bpf->writePage(ph) , RC_SUCCESS);
	fstat(bpf->fd, &s);
    ASSERT_EQ(s.st_size , 31*PAGE_SIZE);

    // overwrite contentPage
    for(int k=0; k<PAGE_SIZE; k++) {
        memset(filler+k, 255, 1);
    }
    ph.image = &filler;
    ph.pid = 8;
    ASSERT_EQ(bpf->writePage(ph) , RC_SUCCESS);

    delete bpf;

    // read back, make sure all data is correct
    bpf = new BitmapPagedFile("test1.bin", BitmapPagedFile::F_NO_CREATE);
    ASSERT_TRUE(bpf);
    for(int k=0; k<8*PAGE_SIZE; k++) {
        if(k == 20) {
            ph.pid = 20;
            ASSERT_EQ(bpf->readPage(ph) , RC_SUCCESS);
            for(int k=0; k<PAGE_SIZE; k++) {
                ASSERT_EQ((*(ph.image))[k] , k%256);
            }
        }
        else if(k == 29) {
            ph.pid = 29;
            ASSERT_EQ(bpf->readPage(ph) , RC_SUCCESS);
            for(int k=0; k<PAGE_SIZE; k++) {
                ASSERT_EQ((*(ph.image))[k] , k%256);
            }
        }
        else if(k == 8) {
            ph.pid = 8;
            ASSERT_EQ(bpf->readPage(ph) , RC_SUCCESS);
            for(int k=0; k<PAGE_SIZE; k++) {
                ASSERT_EQ((*(ph.image))[k] , 255);
            }
        }
        else {
            ph.pid = k;
            ASSERT_EQ(bpf->readPage(ph) , RC_FAILURE);
        }
    }

    delete bpf;
remove("test1.bin");
}

TEST(BmpPagedFile, allocate)
{

    BitmapPagedFile *bpf = new BitmapPagedFile("test1.bin", BitmapPagedFile::F_CREATE);
    ASSERT_TRUE(bpf);

    bpf->allocate(6);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==0){
            ASSERT_EQ((int)bpf->header[k] , 64);
        }
        else
            ASSERT_EQ((int)bpf->header[k] , 0);
    }

    bpf->allocate(15);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==0)
            ASSERT_EQ((int)bpf->header[k] , 64);
        else if(k==1){
            ASSERT_EQ((int)bpf->header[k] , 128);
        }
        else
            ASSERT_EQ((int)bpf->header[k] , 0);
    }

    bpf->allocate(64);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==0)
            ASSERT_EQ((int)bpf->header[k] , 64);
        else if(k==1)
            ASSERT_EQ((int)bpf->header[k] , 128);
        else if(k==8)
            ASSERT_EQ((int)bpf->header[k] , 1);
        else
            ASSERT_EQ((int)bpf->header[k] , 0);
    }

    bpf->allocate(65);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==0)
            ASSERT_EQ((int)bpf->header[k] , 64);
        else if(k==1)
            ASSERT_EQ((int)bpf->header[k] , 128);
        else if(k==8)
            ASSERT_EQ((int)bpf->header[k] , 3);
        else
            ASSERT_EQ((int)bpf->header[k] , 0);
    }

    bpf->allocate(64);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==0)
            ASSERT_EQ((int)bpf->header[k] , 64);
        else if(k==1)
            ASSERT_EQ((int)bpf->header[k] , 128);
        else if(k==8)
            ASSERT_EQ((int)bpf->header[k] , 3);
        else
            ASSERT_EQ((int)bpf->header[k] , 0);
    }

    bpf->allocate(8*PAGE_SIZE);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==0)
            ASSERT_EQ((int)bpf->header[k] , 64);
        else if(k==1)
            ASSERT_EQ((int)bpf->header[k] , 128);
        else if(k==8)
            ASSERT_EQ((int)bpf->header[k] , 3);
        else
            ASSERT_EQ((int)bpf->header[k] , 0);
    }
    /* 
       bpf->allocate(-1);
       for(int k=0; k<PAGE_SIZE; k++) {
       if(k,0)
       ASSERT_EQ((int)bpf->header[k] , 64);
       else if(k,1)
       ASSERT_EQ((int)bpf->header[k] , 128);
       else if(k,8)
       ASSERT_EQ((int)bpf->header[k] , 3);
       else
       ASSERT_EQ((int)bpf->header[k] , 0);
       }*/

    delete bpf;
remove("test1.bin");
}

TEST(BmpPagedFile, deallocate)
{

    BitmapPagedFile *bpf = new BitmapPagedFile("test1.bin", BitmapPagedFile::F_CREATE);
    ASSERT_TRUE(bpf);
    memset(bpf->header, 255, PAGE_SIZE);
    for(int k=0; k<PAGE_SIZE; k++) {
        ASSERT_EQ((int)bpf->header[k] , 255);
    }

    bpf->deallocate(0);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==0)
            ASSERT_EQ((int)bpf->header[k] , 254);
        else
            ASSERT_EQ((int)bpf->header[k] , 255);
    }

    bpf->deallocate(3);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==0)
            ASSERT_EQ((int)bpf->header[k] , 246);
        else
            ASSERT_EQ((int)bpf->header[k] , 255);
    }

    bpf->deallocate(16);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==0)
            ASSERT_EQ((int)bpf->header[k] , 246);
        else if(k==2)
            ASSERT_EQ((int)bpf->header[k] , 254);
        else
            ASSERT_EQ((int)bpf->header[k] , 255);
    }

    bpf->deallocate(3);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==0)
            ASSERT_EQ((int)bpf->header[k] , 246);
        else if(k==2)
            ASSERT_EQ((int)bpf->header[k] , 254);
        else
            ASSERT_EQ((int)bpf->header[k] , 255);
    }

    bpf->deallocate(8*PAGE_SIZE);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==0)
            ASSERT_EQ((int)bpf->header[k] , 246);
        else if(k==2)
            ASSERT_EQ((int)bpf->header[k] , 254);
        else
            ASSERT_EQ((int)bpf->header[k] , 255);
    }

    /*
       bpf->deallocate(-1);
       for(int k=0; k<PAGE_SIZE; k++) {
       if(k,0)
       ASSERT_EQ((int)bpf->header[k] , 246);
       else if(k,2)
       ASSERT_EQ((int)bpf->header[k] , 254);
       else
       ASSERT_EQ((int)bpf->header[k] , 255);
       }*/

    delete bpf;
remove("test1.bin");
}

TEST(BmpPagedFile, isAllocated)
{
    BitmapPagedFile *bpf = new BitmapPagedFile("test1.bin", BitmapPagedFile::F_CREATE);
    ASSERT_TRUE(bpf);
    memset(bpf->header, 255, PAGE_SIZE);

    for(int k=0; k<8*PAGE_SIZE; k++) {
        ASSERT_TRUE(bpf->isAllocated(k));
    }

    for(int k=0; k<rand()%16 + 3; k++) {
        int index = rand()%(8*PAGE_SIZE);
        bpf->deallocate(index);
        ASSERT_FALSE(bpf->isAllocated(index));
        bpf->allocate(index);
        ASSERT_TRUE(bpf->isAllocated(index));
    }

    delete bpf;
remove("test1.bin");
}
/*
void clear_files() {
	remove("test1.bin");
	remove("test2.bin");
}

int main() {
    test_constructor();

    clear_files();
    test_allocate();

    clear_files();
    test_deallocate();

    clear_files();
    test_isAllocated();

    clear_files();
    test_allocatePage();

    clear_files();
    test_allocatePageWithPID();

    clear_files();
    test_disposePage();

    clear_files();
    test_writePage_readPage();

//    remove("test1.bin");
 //   remove("test2.bin");

    return 0;
}*/
