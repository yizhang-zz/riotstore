#include "BtreeIntBlock.h"
#include "Btree.h"
#include <iostream>

BtreeIntBlock::BtreeIntBlock(PageHandle *pPh, Key_t beginsAt, Key_t endsBy,
                             bool create)
    : BtreeSparseBlock(pPh, beginsAt, endsBy, create)
{
    u8 *pData = *ph.image;
    payload = pData+3;

    if (create) {
        u8 *flag = *ph.image;
        *flag = 0;
    }
}

/*
int BtreeIntBlock::search(Key_t key, int &index)
{
    int ret = BtreeSparseBlock::search(key, index);
    if (index == 0)
        return ret;
    if (ret == BT_NOT_FOUND)
        index--;
    return ret;
}
*/

void BtreeIntBlock::print(int depth, Btree *tree)
{
    using namespace std;
    Key_t *pK = (Key_t*) payload;
    PID_t *pP = (PID_t*) (*ph.image + PAGE_SIZE - sizeof(PID_t));
    for (int i=0; i<depth; i++)
        cout<<"  ";
    cout<<"|_";
    cout<<" I["<<lower<<","<<upper<<")\t{";
    for (int i=0; i<*nEntries; i++)
        cout<<pK[i]<<", ("<<pP[-i]<<"), ";
    cout<<"}"<<endl;

    if (tree != NULL && *nEntries > 0) {
        PageHandle ph;
        int i = 0;
        for (; i<*nEntries-1; i++) {
            ph.pid = pP[-i];
            tree->loadPage(ph);
            BtreeBlock *child = BtreeBlock::load(&ph, pK[i], pK[i+1]);
            child->print(depth+1, tree);
            delete child;
            tree->releasePage(ph);
        }
        ph.pid = pP[-i];
        tree->loadPage(ph);
        BtreeBlock *child = BtreeBlock::load(&ph, pK[i], upper);
        child->print(depth+1, tree);
        delete child;
        tree->releasePage(ph);
    }
}

BtreeBlock* BtreeIntBlock::pack()
{
    return NULL;
}
