#ifndef CONFIG_H
#define CONFIG_H
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

    int disableDenseLeaf;
    //int BSplitterBoundary; // boundary for the B splitter
    double TSplitterThreshold; // threshold for the T splitter
    char internalSplitter;
    char leafSplitter;

    int btreeBufferSize;
    int dmaBufferSize;

    Btree::BatchMethod batchMethod;
    u32 batchBufferSize;
    int batchUseHistogram;
    int batchHistogramNum;
    u16 batchKeepPidCount;

    int matmulBlockFactor;

    int BSplitterBoundary() const
    {
        if (disableDenseLeaf)
            return sparseLeafCapacity;
        else
            return denseLeafCapacity;
    }
private:
    Config(const char *path);
};

extern Config *config;

#endif
