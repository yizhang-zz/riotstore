#include "../../common/common.h"
#include "../LRUPageReplacer.h"
#include "../BufferManager.h"
#include "../BitmapPagedFile.h"
#include <assert.h>
#include <stdio.h>
#include <iostream>
using namespace std;

BitmapPagedFile* bpf;
BufferManager<>* bm;
#define FILE_NAME "a.bin"
#define BUFFER_SIZE 2

void create() {
	RC_t rc = BitmapPagedFile::createPagedFile(FILE_NAME, bpf);
	assert(rc == RC_SUCCESS);
	bm = new BufferManager<>(bpf, BUFFER_SIZE);
	assert(bm != NULL);
	cout<<"create test passed"<<endl;
}

void allocate() {
	PageHandle ph;
	PageImage* image;
	for (int i=0; i<3; i++) {
	assert(RC_SUCCESS == bm->allocatePage(ph));
	cout<<ph.pid<<" "<< ph.image<<endl;
	image = ph.image;
	(*image)[0] = i;
	bm->markPageDirty(ph);
	bm->flushPage(ph);
	bm->unpinPage(ph);
	}
	cout<<"allocate test passed"<<endl;
}

void read() {
	PageHandle ph;
	ph.pid = 1;
	bm->readPage(ph);
	assert(*ph.image[0] != 0);
	bm->unpinPage(ph);
	cout<<"read test passed"<<endl;
}

void replaceDirty() {
	PageHandle ph;
	Byte_t updated;
	void *addr;
	
	ph.pid = 0;
	bm->readPage(ph);
	addr = ph.image;
	updated = ++(*(ph.image)[0]);
	bm->markPageDirty(ph);
	bm->unpinPage(ph);
	
	ph.pid = 1;
	bm->readPage(ph);
	++(*(ph.image)[0]);
	bm->markPageDirty(ph);
	bm->unpinPage(ph);

	ph.pid = 2;
	bm->readPage(ph);
	assert(addr == ph.image);
	++(*(ph.image)[0]);
	bm->markPageDirty(ph);
	bm->unpinPage(ph);

	
	// check if page 0 has been flushed
	FILE *f = fopen(FILE_NAME, "r");
	// skip the first header page
	fseek(f, 1*PAGE_SIZE, SEEK_SET);
	char buf;
	fread(&buf, 1, sizeof(char), f);
	fclose(f);
	assert(updated == buf);

	cout<<"replace dirty test passed"<<endl;
}
	
	
void cleanup() {
	delete bm;
	delete bpf;
	cout<<"cleanup test passed"<<endl;
}

int main()
{
	create();
	if (bpf->numContentPages == 0)
		allocate();
	read();
	replaceDirty();
	cleanup();
	return 0;
}

