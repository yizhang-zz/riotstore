#include "BtreeConfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BtreeConfig *BtreeConfig::config = NULL;

BtreeConfig::BtreeConfig(const char *path)
{
    FILE *f = fopen(path, "r");
    if (f == NULL)
        return;
    int ret;
    char buf[512];
    while (fgets(buf, 512, f)) {
        char *a = strtok(buf, "=");
        char *b = strtok(NULL, "\n");
        params[a] = b;
    }
    fclose(f);
}

BtreeConfig* BtreeConfig::getInstance()
{
    if (config)
        return config;
    char buf[512];
    strcpy(buf, getenv("HOME"));
    strcat(buf, "/.riot");
    config = new BtreeConfig(buf);
    return config;
}

void BtreeConfig::dispose()
{
    if (config) {
        delete config;
        config = NULL;
    }
}

const char* BtreeConfig::get(const char* key)
{
    map<string, string>::iterator it = params.find(key);
    if (it != params.end()) {
        return it->second.c_str();
    }
    return NULL;
}
