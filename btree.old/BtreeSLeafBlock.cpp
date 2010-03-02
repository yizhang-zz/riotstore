#include "BtreeSLeafBlock.h"
#include "BtreeDLeafBlock.h"
#include <iostream>

BtreeSLeafBlock::BtreeSLeafBlock(PageHandle *pPh, Key_t beginsAt, Key_t endsBy,
                                 bool create)
    : BtreeSparseBlock(pPh, beginsAt, endsBy, create)
{
    u8 *pData = *ph.image;
    nextLeaf = (PID_t*) (pData+3);
    payload = pData+7;

    if (create) {
        u8 *flag = *ph.image;
        *flag = 1;
    }
}


void BtreeSLeafBlock::print(int depth, Btree *tree)
{
    using namespace std;
    Key_t *pK = (Key_t*) payload;
    Datum_t *pD = (Datum_t*) (*ph.image + PAGE_SIZE - sizeof(Datum_t));
    for (int i=0; i<depth; i++)
        cout<<"  ";
    cout<<"|_";
    cout<<" S["<<lower<<","<<upper<<")\t{";
    for (int i=0; i<*nEntries; i++)
        cout<<"("<<pK[i]<<","<<pD[-i]<<"), ";
    cout<<"}"<<endl;
}

BtreeBlock* BtreeSLeafBlock::pack()
{
    Key_t min = getKey(0);
    Key_t max = getKey(*nEntries-1);
    if (max-min <= BtreeDLeafBlock::capacity) {
        PageImage image;
        PageHandle temp = ph;
        memcpy(image, *ph.image, PAGE_SIZE);
        ph.image = &image;
        u8 *pData = *ph.image;
        nEntries = (u16*) (pData+1);
        nextLeaf = (PID_t*) (pData+3);
        payload = pData+7;
        
        BtreeDLeafBlock* block = new BtreeDLeafBlock(&temp, lower, upper, true);
        Entry e;
        get(0, e);
        Key_t firstKey = e.key;
        for (int i=0; i<*nEntries; i++) {
            get(i, e);
            block->put(e.key-firstKey, e);
        }
        block->setNextLeaf(*nextLeaf);
        return block;
    }
    return NULL;
}
