#include "Config.h"
#include "btree/BtreeDenseLeafBlock.h"
#include "btree/BtreeSparseBlock.h"
#include "directly_mapped/DMABlock.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Config *config = Config::getGlobalConfig();

Config::Config(const char *path)
{
    bmm["NONE"] = Btree::kNone;
    bmm["ALL"] = Btree::kFWF;
    bmm["LRU"] = Btree::kLRU;
    bmm["LP"] = Btree::kLS;
    bmm["SP"] = Btree::kSP;
    bmm["LPP"] = Btree::kLS_RAND;
    bmm["LG"] = Btree::kLG;
    bmm["LGP"] = Btree::kLG_RAND;
    bmm["LPALL"] = Btree::kLPALL;

	denseLeafCapacity = (PAGE_SIZE-sizeof(Btree::DenseLeafBlock::Header))
		/ sizeof(Datum_t);
	sparseLeafCapacity = (PAGE_SIZE-sizeof(Btree::SparseLeafBlock::Header))
		/ (sizeof(Datum_t) + sizeof(Key_t));
	internalCapacity = (PAGE_SIZE-sizeof(Btree::InternalBlock::Header))
		/ (sizeof(PID_t)+sizeof(Key_t));
    dmaBlockCapacity = (PAGE_SIZE-sizeof(DMABlock::Header))/sizeof(Datum_t);

    useDenseLeaf = 1;
	//BSplitterBoundary = denseLeafCapacity;
	TSplitterThreshold = 0.7;
	internalSplitter = 'M';
	leafSplitter = 'B';

	btreeBufferSize = 10;
	dmaBufferSize = 10;

	batchBufferSize = 100;
	batchMethod = Btree::kNone;
	batchUseHistogram = 0;
	batchHistogramNum = 100;
	batchKeepPidCount = 10;
	
    FILE *f = fopen(path, "r");
    if (f != NULL) {
		char buf[512];
		while (fgets(buf, 512, f)) {
			//TODO: trim first
			if (buf[0]=='#')
				continue;
			char *a = strtok(buf, "=");
			char *b = strtok(NULL, "\n");
			if (strcmp(a, "denseLeafCapacity") == 0) {
				denseLeafCapacity = atoi(b);
			} else if (strcmp(a, "sparseLeafCapacity") == 0) {
				sparseLeafCapacity = atoi(b);
			} else if (strcmp(a, "internalCapacity") == 0) {
				internalCapacity = atoi(b);
			} else if (strcmp(a, "dmaBlockCapacity") == 0) {
				dmaBlockCapacity = atoi(b);
			} else if (strcmp(a, "btreeBufferSize") == 0) {
				btreeBufferSize = atoi(b);
			} else if (strcmp(a, "dmaBufferSize") == 0) {
				dmaBufferSize = atoi(b);
			} else if (strcmp(a, "internalSplitter") == 0) {
			    internalSplitter = *b;
			} else if (strcmp(a, "leafSplitter") == 0) {
			    leafSplitter = *b;
			} else if (strcmp(a, "TSplitterThreshold") == 0) {
				TSplitterThreshold = atof(b);
			} else if (strcmp(a, "batchBufferSize") == 0) {
				batchBufferSize = atoi(b);
			} else if (strcmp(a, "batchKeepPidCount") == 0) {
				batchKeepPidCount = atoi(b);
			} else if (strcmp(a, "batchMethod") == 0) {
                batchMethod = bmm.at(b);
			} else if (strcmp(a, "batchUseHistogram") == 0) {
				batchUseHistogram = atoi(b);
			} else if (strcmp(a, "batchHistogramNum") == 0) {
				batchHistogramNum = atoi(b);
			} else if (strcmp(a, "matmulBlockFactor") == 0) {
                matmulBlockFactor = atoi(b);
            } else if (strcmp(a, "useDenseLeaf") == 0) {
                useDenseLeaf = atoi(b);
                //if (disableDenseLeaf)
                //    BSplitterBoundary = sparseLeafCapacity;
            } else if (strcmp(a, "flushPageMinSize") == 0) {
                flushPageMinSize = atoi(b);
            }
		}
		fclose(f);
	}
}

Config* Config::getGlobalConfig()
{
    char buf[512];
	if (getenv("RIOT_CONFIG") != NULL) {
		strcpy(buf, getenv("RIOT_CONFIG"));
	}
	else {
		strcpy(buf, getenv("HOME"));
		strcat(buf, "/.riot");
	}
    return new Config(buf);
}
