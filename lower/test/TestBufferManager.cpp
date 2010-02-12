#include "../../common/common.h"
#include "../BufferManager.h"
#include "../PageReplacer.h"
#include "../BitmapPagedFile.h"
#include <gtest/gtest.h>
#include <assert.h>
#include <stdio.h>
#include <iostream>
using namespace std;

//BitmapPagedFile* bpf;
//BufferManager* bm;
//#define FILE_NAME "a.bin"
#define BUFFER_SIZE 2


TEST(BufferManager, Allocate)
{
    BitmapPagedFile bpf("a.bin", PagedStorageContainer::F_CREATE);
	BufferManager bm(&bpf, BUFFER_SIZE);
	PageHandle ph;
	Byte_t *image;
	for (int i=0; i<3; i++) {
        ASSERT_EQ(RC_OK, bm.allocatePage(ph));
        image = (Byte_t*) bm.getPageImage(ph);
        image[0] = i;
        bm.markPageDirty(ph);
        bm.flushPage(ph);
        bm.unpinPage(ph);
	}
}

TEST(BufferManager, AllocateRead) {
    BitmapPagedFile bpf("a.bin", PagedStorageContainer::F_NO_CREATE);
	BufferManager bm(&bpf, BUFFER_SIZE);
	PageHandle ph;
	Byte_t *image;

	ASSERT_EQ(RC_OK, bm.readPage(1, ph));
    image = (Byte_t*) bm.getPageImage(ph);
	ASSERT_NE(image[0] , 0);
	bm.unpinPage(ph);
}

TEST(BufferManager, ReplaceDirty) {
    BitmapPagedFile bpf("a.bin", PagedStorageContainer::F_NO_CREATE);
	BufferManager bm(&bpf, BUFFER_SIZE);
	PageHandle ph;
	Byte_t *image;

	Byte_t updated;
	void *addr;
	
	bm.readPage(0, ph);
	addr = image = (Byte_t*) bm.getPageImage(ph);
	updated = ++(image[0]);
	bm.markPageDirty(ph);
	bm.unpinPage(ph);
	
	bm.readPage(1, ph);
    image = (Byte_t*) bm.getPageImage(ph);
	++(image[0]);
	bm.markPageDirty(ph);
	bm.unpinPage(ph);

	bm.readPage(2, ph);
	ASSERT_EQ(addr, bm.getPageImage(ph));
    image = (Byte_t*) bm.getPageImage(ph);
	++(image[0]);
	bm.markPageDirty(ph);
	bm.unpinPage(ph);

	
	// check if page 0 has been flushed
	FILE *f = fopen("a.bin", "r");
	// skip the first header page
	fseek(f, 1*PAGE_SIZE, SEEK_SET);
	char buf;
	fread(&buf, 1, sizeof(char), f);
	fclose(f);
    ASSERT_EQ(updated , buf);

    remove("a.bin");
}
