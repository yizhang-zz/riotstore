#include "Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Config *config = Config::getGlobalConfig();

Config::Config(const char *path)
{
	denseLeafCapacity = (PAGE_SIZE-denseLeafHeaderSize)/(sizeof(Datum_t));
	sparseLeafCapacity = (PAGE_SIZE-sparseLeafHeaderSize)/(sizeof(Datum_t)+sizeof(Key_t)+2);
	internalCapacity = (PAGE_SIZE-internalHeaderSize)/(sizeof(PID_t)+sizeof(Key_t)+2);
	btreeBufferSize = 10;
	dmaBufferSize = 10;
	TThreshold = 0.7;
	batchBufferSize = 100;
	batchMethod = Btree::kNone;
	batchUseHistogram = 0;
	batchHistogramNum = 100;
	
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
			} else if (strcmp(a, "btreeBufferSize") == 0) {
				btreeBufferSize = atoi(b);
			} else if (strcmp(a, "dmaBufferSize") == 0) {
				dmaBufferSize = atoi(b);
			} else if (strcmp(a, "TThreshold") == 0) {
				TThreshold = atof(b);
			} else if (strcmp(a, "batchBufferSize") == 0) {
				batchBufferSize = atoi(b);
			} else if (strcmp(a, "batchMethod") == 0) {
				if (strcmp(b, "NONE") == 0)
					batchMethod = Btree::kNone;
				else if(strcmp(b, "FWF") == 0)
					batchMethod = Btree::kFWF;
				else if(strcmp(b, "LRU") == 0)
					batchMethod = Btree::kLRU;
				else if(strcmp(b, "LS") == 0)
					batchMethod = Btree::kLS;
				else if(strcmp(b, "LS_RAND") == 0)
					batchMethod = Btree::kLS_RAND;
				else if(strcmp(b, "LS_RANDCUT") == 0)
					batchMethod = Btree::kLS_RANDCUT;
				else if(strcmp(b, "LG") == 0)
					batchMethod = Btree::kLG;
				else if(strcmp(b, "LG_RAND") == 0)
					batchMethod = Btree::kLG_RAND;
			} else if (strcmp(a, "batchUseHistogram") == 0) {
				batchUseHistogram = atoi(b);
			} else if (strcmp(a, "batchHistogramNum") == 0) {
				batchHistogramNum = atoi(b);
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
