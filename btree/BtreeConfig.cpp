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
			}
		}
		fclose(f);
	}
}

BtreeConfig* BtreeConfig::getGlobalConfig()
{
    char buf[512];
    strcpy(buf, getenv("HOME"));
    strcat(buf, "/.riot");
    return new BtreeConfig(buf);
}
