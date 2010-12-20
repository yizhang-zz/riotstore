#include "common/common.h"
#include "lower/BufferManager.h"
#include "lower/PageReplacer.h"
#include "lower/BitmapPagedFile.h"
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
    BitmapPagedFile bpf("a.bin", PagedStorageContainer::CREATE);
	BufferManager bm(&bpf, BUFFER_SIZE);
	for (int i=0; i<3; i++) {
		PageHandle ph;
        ASSERT_EQ(RC_OK, bm.allocatePageWithPID(i, ph));
		ph->getImage()[0] = i;
		ph->markDirty();
	}
}

TEST(BufferManager, AllocateRead) {
    BitmapPagedFile bpf("a.bin", 0);
	BufferManager bm(&bpf, BUFFER_SIZE);
	for (int i=0; i<3; i++) {
		PageHandle ph;
        ASSERT_EQ(RC_OK, bm.readPage(i, ph));
		ASSERT_EQ(i, ph->getImage()[0]);
	}
}

TEST(BufferManager, ReplaceDirty) {
    BitmapPagedFile bpf("a.bin", 0);
	BufferManager bm(&bpf, BUFFER_SIZE);
	PageHandle ph;
	Byte_t *image;

	Byte_t updated;
	void *addr;
	
	bm.readPage(0, ph);
	addr = image = (Byte_t*) ph->getImage();
	updated = ++(image[0]);
	ph->markDirty();
	
	bm.readPage(1, ph);
    image = (Byte_t*) ph->getImage();
	++(image[0]);
	ph->markDirty();

	bm.readPage(2, ph);
    image = (Byte_t*) ph->getImage();
	ASSERT_EQ(addr, image);
	++(image[0]);
	ph->markDirty();
	
	ph.reset();  // destroys the page
	
	// check if page 0 has been flushed
	FILE *f = fopen("a.bin", "r");
	// skip the header pages
	fseek(f, BitmapPagedFile::HEADER_SIZE, SEEK_SET);
	char buf;
	fread(&buf, 1, sizeof(char), f);
	fclose(f);
    ASSERT_EQ(updated , buf);

    remove("a.bin");
}
