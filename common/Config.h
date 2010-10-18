#pragma once

#include <string>
#include <map>
#include "common.h"

class Config
{
public:
	static Config* getGlobalConfig();
    
	const static int denseLeafHeaderSize = 16;
	const static int sparseLeafHeaderSize = 12;
	const static int internalHeaderSize = 8;

	u16 denseLeafCapacity;
	u16 sparseLeafCapacity;
	u16 internalCapacity;

	int btreeBufferSize;
	int dmaBufferSize;

private:
    Config(const char *path);
};

extern Config *config;

