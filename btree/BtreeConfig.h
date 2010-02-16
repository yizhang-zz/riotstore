#ifndef BTREE_CONFIG_H
#define BTREE_CONFIG_H

#include <string>
#include <map>
using namespace std;

class BtreeConfig
{
public:
    static BtreeConfig* getInstance();
    static void dispose();

    const char* get(const char* key);
    
private:
    static BtreeConfig* config;
    map<string, string> params;
    BtreeConfig(const char *path);
};

#endif
