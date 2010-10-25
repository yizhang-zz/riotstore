#include "LeafHist.h"
#include <iostream>

using namespace Btree;

void LeafHist::locate(Key_t key, HistPageId &pageId)
{
	//TODO: The insertion of a new leaf may cause node boundaries shift and thus change the node membership of already-in-buffer records.
	pageId.hist = this;
	int i = pageId.partId = key/partSize();
	if (hists[i].numLeaves == 0)
		pageId.nodeId = 0;
	else
		pageId.nodeId = (key-hists[i].beginsAt)/(1+
				(hists[i].endsBy-hists[i].beginsAt-1)/hists[i].numLeaves);
}
bool LeafHist::contains(HistPageId *page, Key_t key)
{
	int i = page->partId;
	if (hists[i].beginsAt > key || hists[i].endsBy <= key)
		return false;
	return (hists[i].numLeaves == 0 && page->nodeId == 0)
		|| page->nodeId == (key-hists[i].beginsAt)/(1+
			(hists[i].endsBy-hists[i].beginsAt-1)/hists[i].numLeaves);
}

void LeafHist::print()
{
	using namespace std;
	for (int i=0; i<numParts; ++i) {
		cout<<hists[i].numLeaves<<" ("<<hists[i].endsBy<<") ";
	}
	cout<<endl;
}
