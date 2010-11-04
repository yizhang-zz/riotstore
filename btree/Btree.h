#pragma once

#include "lower/LinearStorage.h"
#include "BtreeBlock.h"
#include "BtreeCursor.h"
#include "Splitter.h"
#include "BatchBuffer.h"
#include "LeafHist.h"
#include <vector>
#ifdef DTRACE_SDT
#include "riot.h"
#endif

namespace Btree
{

#ifdef USE_BATCH_BUFFER
class BatchBuffer;
class BoundPageId;
class HistPageId;
class LeafHist;
#endif


	struct BtreeHeader {
		u32 endsBy;		/// all keys >=0 and < endsBy
		u32 nLeaves;	/// number of leaf nodes
		int depth;		/// depth of tree; root is at depth 0
		PID_t root;		/// PID of root
		PID_t firstLeaf;	/// PID of first leaf node
	};

	/*
	 * A BTree is used to store (key,val) pairs in a one-dimensional array. The
	 * keys starts from 0. It is stored in a paged storage container, e.g.,
	 * BitmapPagedFile. The first (PID=0) page in the file contains the header
	 * of the entire BTree, as defined in struct BTreeHeader.
	 */
	class BTree : public LinearStorage {
	private:
		LeafSplitter *leafSplitter;
		InternalSplitter *internalSplitter;
		// the header block(page) is kept in mem during lifetime of the tree
		// BTreeBlock *headerBlock;
		PageHandle headerPage;
		static const PID_t headerPID = 0;
  
		void split(Cursor *cursor);
  
		PID_t lastPageInBatch; // for batch insertion
		int   lastPageCapacity;
		int   lastPageChange;

	public:
		// points to the data in the header page
		BtreeHeader *header;
#ifdef USE_BATCH_BUFFER
		BatchBuffer *batbuf;
		LeafHist *leafHist;
#endif

		/**
		 * Creates a new BTree with the given file name and dimension
		 * size. If the file exists, it is overwritten. Given dim, all
		 * keys in the BTree should be >=0 and < endsby. The header page
		 * (0-th page in the file) is written immediately.
		 */
		BTree(const char *fileName, u32 endsBy, LeafSplitter *leafSp, InternalSplitter *intSp
			  //#ifdef USE_BATCH_BUFFER
			  //, BatchMethod method
			  //#endif
			  );

		/**
		 * Initializes a BTree from the given file. Metadata is read from the
		 * header page.
		 */
		BTree(const char *fileName, LeafSplitter *leafSp, InternalSplitter *intSp
			  //#ifdef USE_BATCH_BUFFER
			  //,BatchMethod method
			  //#endif
			  );
		/**
		 * Destroys the BTree object (also releases the header page).
		 */
		~BTree();

		// cursor can be a valid path, in which case the search is
		// incremental (first up the tree and then down)
		int search(Key_t key, Cursor *cursor);

#ifdef USE_BATCH_BUFFER
		// finds the PID of the leaf block where key should go into
		void locate(Key_t key, BoundPageId &pageId);
		void locate(Key_t key, HistPageId &pageId);
#endif
		//void locate(Key_t key, PID_t &pid, Key_t &lower, Key_t &upper);

		int put(const Key_t &key, const Datum_t &datum);
		int get(const Key_t &key, Datum_t &datum);

		// Iterator iterates over a collection of type Entry
		template<class Iterator>
		int put(Iterator begin, Iterator end)
		{
			Cursor cursor(buffer);
			int ret = 0;
			for (; begin != end; ++begin) {
				search(begin->key, &cursor);
				putHelper(begin->key, begin->datum, &cursor);
				++ret;
			}
			return ret;
		}

		ArrayInternalIterator *createIterator(IteratorType t, Key_t &beginsAt, Key_t &endsBy);

		void setInternalSplitter(InternalSplitter *sp) {internalSplitter = sp; }
		void setLeafSplitter(LeafSplitter *sp) { leafSplitter = sp; }

		void print();
#ifdef USE_BATCH_BUFFER
		void flushAndPrint()
		{
			if (batbuf)
				batbuf->flushAll();
			print();
		}
#endif
		u32  getNumLeaves() { return header->nLeaves; }
		Key_t getUpperBound() { return header->endsBy; }
		/*
		void allocatePage(PID_t &pid, PageHandle &ph) {
			buffer->allocatePage(ph);
			pid = buffer->getPID(ph);
		}
		PageHandle loadPage(PID_t pid) {
			PageHandle ph;
			buffer->readPage(pid, ph);
			return ph;
		}
		void releasePage(PageHandle ph) {
			buffer->unpinPage(ph);
		}
		char *getPageImage(PageHandle ph) {
			return (char*)buffer->getPageImage(ph);
		}
		*/

	private:
		void init(const char *fileName, int fileFlag);
#ifdef USE_BATCH_BUFFER
		void initBatching();
#endif
		void print(PID_t pid, Key_t beginsAt, Key_t endsBy, int depth);
		int putHelper(Key_t key, Datum_t datum, Cursor *cursor);

		void onNewLeaf(Block *leaf)
		{
			header->nLeaves++;
#ifdef USE_BATCH_BUFFER
			if (leafHist)
				leafHist->onNewLeaf(leaf->getLowerBound(), leaf->getUpperBound());
#endif
#ifdef DTRACE_SDT
			RIOT_BTREE_SPLIT_LEAF();
#endif
		}
	};
}
