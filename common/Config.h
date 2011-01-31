#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <boost/unordered_map.hpp>
#include "common.h"
#include "btree/BatchBuffer.h"

//enum Btree::BatchMethod;

class Config
{
public:
    static Config* getGlobalConfig();
    
    typedef boost::unordered_map<std::string, Btree::BatchMethod> BatchMethodMap;
    BatchMethodMap bmm;
    //const static int denseLeafHeaderSize = 16;
    //const static int sparseLeafHeaderSize = 12;
    //const static int internalHeaderSize = 8;

    u16 denseLeafCapacity;
    u16 sparseLeafCapacity;
    u16 internalCapacity;
    u16 dmaBlockCapacity;

    int useDenseLeaf;
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

private:
    Config(const char *path);
};

extern Config *config;

#endif
