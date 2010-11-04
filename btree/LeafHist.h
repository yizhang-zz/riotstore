#pragma once

#include "common/common.h"

namespace Btree
{
	class HistPageId;

	class LeafHist
	{
	public:
		class Partition
		{
		public:
			Key_t beginsAt;
			Key_t endsBy;
			u32 numLeaves;
			Partition() : numLeaves(0)
			{
			}
		private:
		};

		Key_t partSize()
		{
			return 1+(endsBy-1)/numParts;
		}

		LeafHist(int numParts_, Key_t endsBy_)
			:numParts(numParts_), endsBy(endsBy_)
		{
			hists = new Partition[numParts];
			hists[0].beginsAt = 0;
			for (int i=1; i<numParts; ++i)
				hists[i].beginsAt = hists[i-1].endsBy = partSize()*i;
			hists[numParts-1].endsBy = endsBy;
		}

		~LeafHist()
		{
			delete[] hists;
		}

		void onNewLeaf(Key_t lower, Key_t upper)
		{
			Key_t index = lower/partSize();
			hists[index].numLeaves++;
		}

		void locate(Key_t key, HistPageId &pageId);
			
		bool contains(HistPageId *page, Key_t key);
		void print();

		int numParts;
		Key_t endsBy;
		Partition *hists;
	};

	class HistPageId
	{
	public:
		int partId;
		u32 nodeId;
		Key_t lower;
		Key_t upper;
		u16 count;
		LeafHist *hist;
		
		HistPageId()
		{
		}

		HistPageId(Key_t l, Key_t u, u16 c): lower(l), upper(u), count(c)
		{
		}

		friend std::ostream & operator<<(std::ostream &out, const HistPageId &page);

		bool contains(Key_t key)
		{
			return hist->contains(this, key);
		}

		bool operator<(const HistPageId &other) const {return lower < other.lower;}
	};

	inline std::ostream & operator<<(std::ostream &out, const Btree::HistPageId &page)
	{
		out<<"<"<<page.partId<<","<<page.nodeId<<">";
		return out;
	}

}

