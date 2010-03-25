#include "BtreePagePacker.h"
#include "SkipList.h"
#include "BtreeBlock.h"

using namespace Btree;

void BtreePagePacker::pack(void *unpacked, void *packed)
{
    u8 *p = (u8*) packed;
    // PageInfo *info = (PageInfo*) unpacked;
    SkipList<Key_t, Value> *list = (SkipList<Key_t, Value>*) unpacked;
    SkipList<Key_t, Value>::Iterator *it = list->getIterator();
    if (p[0] & 2 == 0) { // sparse format
        u8 *payload;
        int skip;
        int size;
        if (p[0] & 1) { // leaf
            payload = p + 8;
            skip = sizeof(Key_t)+sizeof(Datum_t);
            size = sizeof(Datum_t);
        }
        else { // internal
            payload = p + 4;
            size = sizeof(PID_t);
            skip = sizeof(Key_t)+size;
        }
        int i = 0;
        Key_t key;
        Value val;    
        while (it->moveNext()) {
            it->get(key, val);
            memcpy(payload+i*skip, &key, sizeof(Key_t));
            memcpy(payload+i*skip+sizeof(Key_t), &val, size);
            i++;
        }
        u16 num = list->getSize();
        memcpy(p+2, &num, sizeof(u16));
    }
    else { // dense leaf
        Datum_t *payload = (Datum_t*)(p + 12);
        Key_t *headKey = (Key_t*)(p + 8);
        int size = sizeof(Datum_t);
        int skip = sizeof(Key_t)+size;
        Key_t key;
        Value val;
        list->locate(0, key, val);
        int i = key;
        *headKey = key;
        while (it->moveNext()) {
            it->get(key, val);
            while (i < key) {
                payload[i-*headKey] = Block::defaultValue;
                i++;
            }
            payload[i-*headKey] = val.datum;
            i++;
        }
        u16 num = key - *headKey + 1;
        memcpy(p+2, &num, sizeof(u16));
    }
    delete it;
}

//apr_pool_t* BtreePagePacker::pool = NULL;

void BtreePagePacker::unpack(void *packed, void *&unpacked)
{
    //if (pool==NULL)
    //    apr_pool_create(&pool, NULL);
    // NULL argument: create a separate new memory pool, no parent
    SkipList<Key_t, Value> *list = new SkipList<Key_t, Value>(NULL);
    unpacked = list;
    u8 *p = (u8*) packed;
    u16 num = *((u16*)(p+2));
    if ((p[0] & 2) == 0) { // sparse format
        u8 *payload;
        int skip;
        int size;
        if (p[0] & 1) { // leaf
            payload = p + 8;
            skip = sizeof(Key_t)+sizeof(Datum_t);
            size = sizeof(Datum_t);
        }
        else { // internal
            payload = p + 4;
            size = sizeof(PID_t);
            skip = sizeof(Key_t)+size;
        }
        int i = 0;
        Key_t key;
        Value val;
        for (i=0; i<num; i++) {
            key = *((Key_t*)(payload+i*skip));
            memcpy(&val, payload+i*skip+sizeof(Key_t), size);
            list->insert(key, val);
        }
    }
    else { // dense leaf
        Datum_t *payload = (Datum_t*)(p + 12);
        Key_t headKey = *((Key_t*)(p + 8));
        Key_t key;
        Value val;
        for (int i=0; i<num; i++) {
            val.datum = payload[i];
            list->insert(headKey+i, val);
        }
    }
}


void BtreePagePacker::destroyUnpacked(void *&unpacked)
{
    assert(unpacked);
    SkipList<Key_t, Value> *list = (SkipList<Key_t, Value>*) unpacked;
    delete list;
    unpacked = NULL;
}
