#include "../../common/common.h"
#include "../BufferManager.h"
#include "../PageReplacer.h"
#include "../BitmapPagedFile.h"
#include <gtest/gtest.h>
#include <assert.h>
#include <stdio.h>
#include <iostream>
using namespace std;

TEST(BitmapPagedFile, Allocate)
{
	char file[] = "bpf.bin";
	size_t num = BitmapPagedFile::NUM_BITS_HEADER;
	BitmapPagedFile bpf(file, BitmapPagedFile::CREATE);
	for (PID_t i=0; i<num; ++i) {
		ASSERT_EQ(RC_OK, bpf.allocatePageWithPID(i));
	}
	ASSERT_EQ(RC_OutOfRange, bpf.allocatePageWithPID(num));
	ASSERT_EQ(RC_OutOfRange, bpf.disposePage(num));

	const int count = 100;
	PID_t pids[count];
	kPermute(pids, (PID_t)0, (PID_t)num-1, count);
	for (int i=0; i<count; ++i) {
		ASSERT_EQ(RC_OK, bpf.disposePage(pids[i]));
		ASSERT_EQ(RC_NotAllocated, bpf.disposePage(pids[i]));
	}
	for (int i=0; i<count; ++i) {
		ASSERT_EQ(RC_OK, bpf.allocatePageWithPID(pids[i]));
	}
}

TEST(BitmapPagedFile, constructor) 
{
	remove("test1.bin");
    // create new file
    BitmapPagedFile *bpf = new BitmapPagedFile("test1.bin", BitmapPagedFile::CREATE);
	PageRec *rec = new PageRec;
	rec->image = new char[PAGE_SIZE];

	// current pid range in file is [0,0), i.e., empty
    for(rec->pid=0; rec->pid<BitmapPagedFile::NUM_BITS_HEADER; rec->pid++)
        ASSERT_EQ(RC_OutOfRange, bpf->readPage(rec))<<"for pid="<<rec->pid;

	delete[] rec->image;
	delete rec;
    delete bpf;
}

TEST(BitmapPagedFile, allocatePage)
{
	PageRec *rec = new PageRec;
	rec->image = new char[PAGE_SIZE];
	PID_t pid;
    BitmapPagedFile *bpf = new BitmapPagedFile("test1.bin", 0); // open existing
    ASSERT_TRUE(bpf);

	// allocate page 5
	rec->pid = 5;
	bpf->allocatePageWithPID(rec->pid);
    ASSERT_EQ(RC_OK, bpf->readPage(rec));

    for(rec->pid=0; rec->pid<5; rec->pid++)
        ASSERT_EQ(RC_NotAllocated, bpf->readPage(rec))<<"for pid="<<rec->pid;
    for(rec->pid=6; rec->pid<BitmapPagedFile::NUM_BITS_HEADER; rec->pid++)
        ASSERT_NE(RC_OK, bpf->readPage(rec))<<"for pid="<<rec->pid;

    // allocate page 0~3 and 6~9
    for(PID_t k=0; k<4; k++)
        ASSERT_EQ(RC_OK, bpf->allocatePageWithPID(k));
    for(PID_t k=6; k<10; k++)
        ASSERT_EQ(RC_OK, bpf->allocatePageWithPID(k));

	// allocate with specifying pid 
	bpf->allocatePage(pid);
	ASSERT_EQ(10, pid);

    // allocate past range
	pid = BitmapPagedFile::NUM_BITS_HEADER;
    ASSERT_EQ(RC_OutOfRange, bpf->allocatePageWithPID(pid));

    delete bpf;
	delete[] rec->image;
	delete rec;
}

TEST(BitmapPagedFile, allocatedPagePID)
{
    BitmapPagedFile *bpf = new BitmapPagedFile("test1.bin", 0);
    ASSERT_TRUE(bpf);

	// allocate page 100 and verify file size
	ASSERT_EQ(RC_OK, bpf->allocatePageWithPID(100));
	delete bpf; // forces write

	struct stat s;
	stat("test1.bin", &s);
    ASSERT_EQ(s.st_size , (BitmapPagedFile::NUM_HEADER_PAGES+101)*PAGE_SIZE);
}

TEST(BitmapPagedFile, disposePage)
{
	PageRec *rec = new PageRec;
	rec->image = new char[PAGE_SIZE];
    BitmapPagedFile *bpf = new BitmapPagedFile("test1.bin", 0);
    ASSERT_TRUE(bpf);

    ASSERT_EQ(RC_OK, bpf->disposePage(5));
	rec->pid = 5;
	ASSERT_EQ(RC_NotAllocated, bpf->readPage(rec));
    delete bpf;
	delete[] rec->image;
	delete rec;
}

TEST(BitmapPagedFile, readWrite)
{
    PageRec ph;
    char *filler = (char*) allocPageImage(1);
    BitmapPagedFile *bpf = new BitmapPagedFile("test1.bin", 0);
    ASSERT_TRUE(bpf);

    for(size_t k=0; k<PAGE_SIZE; k++) {
		filler[k] = k % 256;
    }
    ph.image = filler;
    ph.pid = 200;

    ASSERT_EQ(bpf->writePage(&ph) , RC_OutOfRange); // pid out of range and
                                                // unallocated
	ASSERT_EQ(RC_OK, bpf->allocatePageWithPID(ph.pid));
	ASSERT_EQ(RC_OK, bpf->writePage(&ph));

    delete bpf;

	struct stat s;
	stat("test1.bin", &s);
	ASSERT_EQ(s.st_size, (BitmapPagedFile::NUM_HEADER_PAGES+ph.pid+1)*PAGE_SIZE)<<s.st_size/PAGE_SIZE;


    // read back, make sure all data is correct
    bpf = new BitmapPagedFile("test1.bin", 0);
    ASSERT_TRUE(bpf);
	ASSERT_EQ(RC_OK, bpf->readPage(&ph));
	for(size_t k=0; k<PAGE_SIZE; k++) {
		ASSERT_EQ(k%256, ((Byte_t*)ph.image)[k]);
	}

    delete bpf;
	freePageImage(filler);
    remove("test1.bin");
}

