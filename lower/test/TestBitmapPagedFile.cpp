
#include "BitmapPagedFile.h"
#include <assert.h>

void test_constructor() {
}

void test_desctructor() {
}

void test_createPagedFile() {
   BitmapPagedFile* bpf;
   assert(createPagedFile("test1.bin", &bpf) == SUCCESS);
}

void test_allocatePage() {
}

void test_allocatePageWithPid() {
}

void test_disposePage() {
}

void test_readPage() {
}

void test_writePage() {
}

void test_allocatePid() {
}

void test_deallocatePid() {
}

void test_isAllocated() {
}

int main() {
   test_createPagedFile();
   test_BitmapPagedFile();
   return 0;
}
