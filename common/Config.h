#pragma once

#include <string>
#include <map>
#include "common.h"
#include "btree/BatchBuffer.h"

//enum Btree::BatchMethod;

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

	double TThreshold; // threshold for the T splitter

	Btree::BatchMethod batchMethod;
	u32 batchBufferSize;
	int batchUseHistogram;
	int batchHistogramNum;
private:
    Config(const char *path);
};

extern Config *config;

