#ifndef BTREE_CONFIG_H
#define BTREE_CONFIG_H

#include <string>
#include <map>
#include "../common/common.h"

namespace Btree
{
class BtreeConfig
{
public:
	static BtreeConfig* getGlobalConfig();
    //static BtreeConfig* getInstance();
    //static void dispose();

    //const char* get(const char* key);
    
	const static int denseLeafHeaderSize = 16;
	const static int sparseLeafHeaderSize = 12;
	const static int internalHeaderSize = 8;

	u16 denseLeafCapacity;
	u16 sparseLeafCapacity;
	u16 internalCapacity;

	int btreeBufferSize;
	int dmaBufferSize;

private:
    //static BtreeConfig* config;
	//std::map<std::string, std::string> params;
    BtreeConfig(const char *path);
};

extern BtreeConfig *config;
};

#endif
