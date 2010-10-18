#include "BtreeConfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Btree;

BtreeConfig *Btree::config = BtreeConfig::getGlobalConfig();

BtreeConfig::BtreeConfig(const char *path)
{
	denseLeafCapacity = (PAGE_SIZE-denseLeafHeaderSize)/(sizeof(Datum_t));
	sparseLeafCapacity = (PAGE_SIZE-sparseLeafHeaderSize)/(sizeof(Datum_t)+sizeof(Key_t)+2);
	internalCapacity = (PAGE_SIZE-internalHeaderSize)/(sizeof(PID_t)+sizeof(Key_t)+2);
	btreeBufferSize = 10;
	dmaBufferSize = 10;
	
    FILE *f = fopen(path, "r");
    if (f != NULL) {
		char buf[512];
		while (fgets(buf, 512, f)) {
			//TODO: trim first
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
			}
		}
		fclose(f);
	}
}

BtreeConfig* BtreeConfig::getGlobalConfig()
{
    char buf[512];
	if (getenv("RIOT_CONFIG") != NULL) {
		strcpy(buf, getenv("RIOT_CONFIG"));
	}
	else {
		strcpy(buf, getenv("HOME"));
		strcat(buf, "/.riot");
	}
    return new BtreeConfig(buf);
}
