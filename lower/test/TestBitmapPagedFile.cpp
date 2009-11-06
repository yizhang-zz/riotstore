
#include <stdlib.h>
#include <string.h>
#include "../../common/common.h"
#include "../BitmapPagedFile.h"
#include <assert.h>
#include <iostream>
using namespace std;

BitmapPagedFile* bpf;
RC_t rc;
PID_t pid;
Byte_t filler[PAGE_SIZE];
FILE* f;
PageHandle ph;

void test_constructor() {
    // create new file
    rc = BitmapPagedFile::createPagedFile("test1.bin", bpf);
    assert(rc == RC_SUCCESS);
    for(int k=0; k<PAGE_SIZE; k++) {
        assert(!bpf->isAllocated(k));
    }
    assert(bpf->numContentPages == 0);

    bpf->allocate(0);
    assert(bpf->header[0] == 1);
    assert(bpf->isAllocated(0));
    delete bpf;

    // open same file
    rc = BitmapPagedFile::createPagedFile("test1.bin", bpf);
    assert(rc == RC_SUCCESS);
    assert(bpf->isAllocated(0));
    assert(bpf->header[0] == 1);
    assert(bpf->numContentPages == 0);
    for(int k=1; k<PAGE_SIZE; k++) {
        assert(!bpf->isAllocated(k));
    }
    delete bpf;

    // open some other file
    rc = BitmapPagedFile::createPagedFile("test2.bin", bpf);
    assert(rc == RC_SUCCESS);
    for(int k=0; k<PAGE_SIZE; k++) {
        assert(!bpf->isAllocated(k));
    }
    delete bpf;

    cout << "createPagedFile passed test cases" << endl;
}

void test_allocatePage() {
    rc = BitmapPagedFile::createPagedFile("test1.bin", bpf);
    assert(rc == RC_SUCCESS);

    // allocate each bit sequentially in header
    for(int k=0; k<8*PAGE_SIZE; k++) {
        assert(bpf->allocatePage(pid) == RC_SUCCESS);
        assert(pid == k);
        assert(bpf->numContentPages == k+1);
    }

    // allocate past range
    assert(bpf->allocatePage(pid) == RC_FAILURE);

    // allocate holes from least to greatest
    bpf->header[3] = 0;
    bpf->header[123] = 0;
    bpf->header[4000] = 0;

    for(int k=0; k<8; k++) {
        assert(bpf->allocatePage(pid) == RC_SUCCESS);
        assert(pid == 3*8+k);
    }
    for(int k=0; k<8; k++) {
        assert(bpf->allocatePage(pid) == RC_SUCCESS);
        assert(pid == 123*8+k);
    }
    for(int k=0; k<8; k++) {
        assert(bpf->allocatePage(pid) == RC_SUCCESS);
        assert(pid == 4000*8+k);
    }
    assert(bpf->allocatePage(pid) == RC_FAILURE);
    delete bpf;
    cout << "allocatePage passed test cases" << endl;
}

void test_allocatePageWithPID() {
    rc = BitmapPagedFile::createPagedFile("test1.bin", bpf);
    assert(rc == RC_SUCCESS);

    // allocate each PID once
    for(int k=0; k<8*PAGE_SIZE; k++) {
        assert(bpf->allocatePageWithPID(k) == RC_SUCCESS);
        assert(bpf->numContentPages == k+1);
        assert(bpf->allocatePageWithPID(k) == RC_FAILURE);
        assert(bpf->numContentPages == k+1);
    }

    assert(bpf->allocatePageWithPID(8*PAGE_SIZE) == RC_FAILURE);
    delete bpf;

    // header and contentPages written and retrieved
    BitmapPagedFile::createPagedFile("test1.bin", bpf);
    fseek(bpf->file, 0, SEEK_END);
    assert(bpf->numContentPages == 8*PAGE_SIZE);
    assert(ftell(bpf->file) == (8*PAGE_SIZE+1)*PAGE_SIZE);
    for(int k=0; k<PAGE_SIZE; k++) {
        assert(bpf->header[k] == 255);
    }
    delete bpf;

    BitmapPagedFile::createPagedFile("test2.bin", bpf);
    assert(bpf->allocatePageWithPID(202) == RC_SUCCESS);
    delete bpf;

    BitmapPagedFile::createPagedFile("test2.bin", bpf);
    assert(bpf->numContentPages == 203);
    fseek(bpf->file, 0, SEEK_END);
    assert(ftell(bpf->file) == (204)*PAGE_SIZE);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==25) {
            assert(bpf->header[k] == 4);
        }
        else
            assert(bpf->header[k] == 0);
    }
    delete bpf;

    cout << "allocatePageWithPID passed test cases" << endl;
}

void test_disposePage() {
    rc = BitmapPagedFile::createPagedFile("test1.bin", bpf);
    assert(rc == RC_SUCCESS);

    bpf->header[4] = 130; // 10000010
    bpf->header[1543] = 47; // 00101111
    bpf->numContentPages = 15000;

    assert(bpf->disposePage(39) == RC_SUCCESS);
    assert(bpf->header[4] == 2);
    assert(bpf->disposePage(12345) == RC_SUCCESS);
    assert(bpf->header[1543] == 45);

    for(int k=0; k<8*PAGE_SIZE; k++) {
        if(k==33||k==12344||k==12346||k==12347||k==12349) {
            assert(bpf->disposePage(k) == RC_SUCCESS);
        }
        else {
            assert(bpf->disposePage(k) == RC_FAILURE);
        }
    }

    // assert(bpf->disposePage(-1) == RC_FAILURE);
    assert(bpf->disposePage(8*PAGE_SIZE) == RC_FAILURE);

    delete bpf;
    cout << "disposePage passed test cases" << endl;
}

void test_writePage_readPage() {
    rc = BitmapPagedFile::createPagedFile("test1.bin", bpf);
    assert(rc == RC_SUCCESS);

    for(int k=0; k<PAGE_SIZE; k++) {
        memset(filler+k, k%256, 1);
    }
    ph.image = &filler;
    ph.pid = 20;

    assert(bpf->writePage(ph) == RC_FAILURE); // pid out of range and
                                                // unallocated

    bpf->header[2] = 16; // 00010000
    bpf->numContentPages = 20;
    assert(bpf->writePage(ph) == RC_FAILURE); // pid out of range

    // pad contentPages on file
    bpf->numContentPages = 30;
    assert(bpf->writePage(ph) == RC_SUCCESS);
    fflush(bpf->file);
    fseek(bpf->file, 0, SEEK_END);\
        assert(ftell(bpf->file) == 22*PAGE_SIZE);

    // write to one of contentPages
    bpf->header[1] = 1; // 00000001
    ph.pid = 8;
    assert(bpf->writePage(ph) == RC_SUCCESS);

    // expand contentPages
    bpf->header[3] = 32; // 00100000
    ph.pid = 29;
    assert(bpf->writePage(ph) == RC_SUCCESS);
    fflush(bpf->file);
    fseek(bpf->file, 0, SEEK_END);
    assert(ftell(bpf->file) == 31*PAGE_SIZE);

    // overwrite contentPage
    for(int k=0; k<PAGE_SIZE; k++) {
        memset(filler+k, 255, 1);
    }
    ph.image = &filler;
    ph.pid = 8;
    assert(bpf->writePage(ph) == RC_SUCCESS);

    delete bpf;

    // read back, make sure all data is correct
    rc = BitmapPagedFile::createPagedFile("test1.bin", bpf);
    assert(rc == RC_SUCCESS);
    for(int k=0; k<8*PAGE_SIZE; k++) {
        if(k == 20) {
            ph.pid = 20;
            assert(bpf->readPage(ph) == RC_SUCCESS);
            for(int k=0; k<PAGE_SIZE; k++) {
                assert((*(ph.image))[k] == k%256);
            }
        }
        else if(k == 29) {
            ph.pid = 29;
            assert(bpf->readPage(ph) == RC_SUCCESS);
            for(int k=0; k<PAGE_SIZE; k++) {
                assert((*(ph.image))[k] == k%256);
            }
        }
        else if(k == 8) {
            ph.pid = 8;
            assert(bpf->readPage(ph) == RC_SUCCESS);
            for(int k=0; k<PAGE_SIZE; k++) {
                assert((*(ph.image))[k] == 255);
            }
        }
        else {
            ph.pid = k;
            assert(bpf->readPage(ph) == RC_FAILURE);
        }
    }

    delete bpf;
    cout << "writePage and readPage passed test cases" << endl;
}

void test_allocate() {
    rc = BitmapPagedFile::createPagedFile("test1.bin", bpf);
    assert(rc == RC_SUCCESS);

    bpf->allocate(6);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==0){
            assert((int)bpf->header[k] == 64);
        }
        else
            assert((int)bpf->header[k] == 0);
    }

    bpf->allocate(15);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==0)
            assert((int)bpf->header[k] == 64);
        else if(k==1){
            assert((int)bpf->header[k] == 128);
        }
        else
            assert((int)bpf->header[k] == 0);
    }

    bpf->allocate(64);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==0)
            assert((int)bpf->header[k] == 64);
        else if(k==1)
            assert((int)bpf->header[k] == 128);
        else if(k==8)
            assert((int)bpf->header[k] == 1);
        else
            assert((int)bpf->header[k] == 0);
    }

    bpf->allocate(65);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==0)
            assert((int)bpf->header[k] == 64);
        else if(k==1)
            assert((int)bpf->header[k] == 128);
        else if(k==8)
            assert((int)bpf->header[k] == 3);
        else
            assert((int)bpf->header[k] == 0);
    }

    bpf->allocate(64);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==0)
            assert((int)bpf->header[k] == 64);
        else if(k==1)
            assert((int)bpf->header[k] == 128);
        else if(k==8)
            assert((int)bpf->header[k] == 3);
        else
            assert((int)bpf->header[k] == 0);
    }

    bpf->allocate(8*PAGE_SIZE);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==0)
            assert((int)bpf->header[k] == 64);
        else if(k==1)
            assert((int)bpf->header[k] == 128);
        else if(k==8)
            assert((int)bpf->header[k] == 3);
        else
            assert((int)bpf->header[k] == 0);
    }
    /* 
       bpf->allocate(-1);
       for(int k=0; k<PAGE_SIZE; k++) {
       if(k==0)
       assert((int)bpf->header[k] == 64);
       else if(k==1)
       assert((int)bpf->header[k] == 128);
       else if(k==8)
       assert((int)bpf->header[k] == 3);
       else
       assert((int)bpf->header[k] == 0);
       }*/

    delete bpf;
    cout << "allocate passed test cases" << endl;
}

void test_deallocate() {
    rc = BitmapPagedFile::createPagedFile("test1.bin", bpf);
    assert(rc == RC_SUCCESS);
    memset(bpf->header, 255, PAGE_SIZE);
    for(int k=0; k<PAGE_SIZE; k++) {
        assert((int)bpf->header[k] == 255);
    }

    bpf->deallocate(0);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==0)
            assert((int)bpf->header[k] == 254);
        else
            assert((int)bpf->header[k] == 255);
    }

    bpf->deallocate(3);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==0)
            assert((int)bpf->header[k] == 246);
        else
            assert((int)bpf->header[k] == 255);
    }

    bpf->deallocate(16);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==0)
            assert((int)bpf->header[k] == 246);
        else if(k==2)
            assert((int)bpf->header[k] == 254);
        else
            assert((int)bpf->header[k] == 255);
    }

    bpf->deallocate(3);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==0)
            assert((int)bpf->header[k] == 246);
        else if(k==2)
            assert((int)bpf->header[k] == 254);
        else
            assert((int)bpf->header[k] == 255);
    }

    bpf->deallocate(8*PAGE_SIZE);
    for(int k=0; k<PAGE_SIZE; k++) {
        if(k==0)
            assert((int)bpf->header[k] == 246);
        else if(k==2)
            assert((int)bpf->header[k] == 254);
        else
            assert((int)bpf->header[k] == 255);
    }

    /*
       bpf->deallocate(-1);
       for(int k=0; k<PAGE_SIZE; k++) {
       if(k==0)
       assert((int)bpf->header[k] == 246);
       else if(k==2)
       assert((int)bpf->header[k] == 254);
       else
       assert((int)bpf->header[k] == 255);
       }*/

    delete bpf;
    cout << "deallocate passed test cases" << endl;
}

void test_isAllocated() {
    rc = BitmapPagedFile::createPagedFile("test1.bin", bpf);
    assert(rc == RC_SUCCESS);
    memset(bpf->header, 255, PAGE_SIZE);

    for(int k=0; k<8*PAGE_SIZE; k++) {
        assert(bpf->isAllocated(k));
    }

    for(int k=0; k<rand()%16 + 3; k++) {
        int index = rand()%(8*PAGE_SIZE);
        bpf->deallocate(index);
        assert(!bpf->isAllocated(index));
        bpf->allocate(index);
        assert(bpf->isAllocated(index));
    }

    delete bpf;
    cout << "isAllocated passed test cases" << endl;
}

void clear_files() {
    f = fopen("test1.bin", "wb+");
    fclose(f);
    f = fopen("test2.bin", "wb+");
    fclose(f);

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

    remove("test1.bin");
    remove("test2.bin");

    return 0;
}
