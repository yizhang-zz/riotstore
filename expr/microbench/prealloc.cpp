#include <time.h>
#include <iostream>
#include <fstream>
#include "common/common.h"
#include "common/Config.h"
#include "btree/Btree.h"

using namespace std;
using namespace Btree;

int main(int argc, char **argv)
{
	if (argc < 3) {
		cerr<<"Usage: "<<argv[0]<<" <file> <matrix scale>"<<endl
			<<"  where <file> is the output file and <matrix scale> is the #rows or cols"<<endl;
		return 1;
	}

	const char *file = argv[1];
	Key_t scale = atoi(argv[2]);
	Key_t total = scale * scale;

	InternalSplitter *isp = new MSplitter<PID_t>();
	LeafSplitter *lsp = new MSplitter<Datum_t>();
	BTree *tree = new BTree(file, total, lsp, isp);
	delete tree;
	delete isp;
	delete lsp;

	int fd = open(file, O_WRONLY);
	assert(fd >= 0);
	unsigned count = 1+(total-1)/config->sparseLeafCapacity;
	lseek(fd, -1, SEEK_END);
	for (unsigned i = 0; i<count; ++i) {
		lseek(fd, PAGE_SIZE, SEEK_CUR);
		write(fd, "a", 1);
	}
	close(fd);
}
