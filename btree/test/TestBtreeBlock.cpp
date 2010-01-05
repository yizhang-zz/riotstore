#include <gtest/gtest.h>
#include "../BtreeBlock.h"

/************************************************
 * Dense leaf block tests.
 */

TEST(BtreeBlock, DenseLeaf_CreateEmpty)
{
	PageImage image;
	PageHandle ph;
	ph.image = &image;
	ph.pid = 0;

    BtreeDLeafBlock block(&ph, 0);
    Key_t lower = block.getLowerBound();
    Key_t upper = block.getUpperBound();
	
	ASSERT_EQ(block.getSize(), 0);
	ASSERT_EQ(lower, 0);
	ASSERT_EQ(upper, block.getCapacity());
	
	Datum_t datum;

	for (int i=lower; i < upper; i++) {
		ASSERT_EQ(block.get(i, &datum), BT_OK);
		ASSERT_TRUE(BtreeBlock::IS_DEFAULT(datum));
	}
}

TEST(BtreeBlock, DenseLeaf_InitExisting)
{
	PageImage image;
	PageHandle ph;
	ph.image = &image;
	ph.pid = 0;

	BtreeDLeafBlock block(&ph, 0);
	Datum_t datum = 1.0;
	block.put(1, &datum);
	block.put(46, &datum);

    Key_t endsBy = block.getCapacity();

	// init another block with existing data
	BtreeBlock *block1 = BtreeBlock::load(&ph, 0, endsBy);
	ASSERT_EQ(block1->getSize(), 2);
	ASSERT_EQ(block1->getLowerBound(), 0);
	ASSERT_EQ(block1->getUpperBound(), block1->getCapacity());
	ASSERT_TRUE(dynamic_cast<BtreeDLeafBlock*>(block1));
    delete block1;
}

TEST(BtreeBlock, DenseLeaf_PutGet)
{
	PageImage image;
	PageHandle ph;
	ph.image = &image;
	ph.pid = 0;

	BtreeDLeafBlock block(&ph, 0);
	Key_t endsBy = block.getCapacity();

	// 5 pairs of key,datum to be inserted
	const int num = 5;
	Key_t idx[] = {0, 11, 25, 67, endsBy-1};
	Datum_t data[] = {1.0, 2.0, 3.0, 4.0, 5.0};
	Datum_t data1[num];
	for (int i=0; i<num; i++)
		data1[i] = data[i] + 1.0;

	// non-overwrite put
	for (int i=0; i<num; i++)
		ASSERT_TRUE(block.put(idx[i], &data[i])==BT_OK);
	
	// read back
	Datum_t datum;
	int k = 0;
	for (int i=block.getLowerBound(); i < block.getUpperBound(); i++) {
		if (idx[k] > i) {
			ASSERT_TRUE(block.get(i, &datum)==BT_OK);
			ASSERT_TRUE(BtreeBlock::IS_DEFAULT(datum))<<" @ "<<i;
		}
		else {
			ASSERT_TRUE(block.get(i, &datum)==BT_OK);
			ASSERT_DOUBLE_EQ(datum, data[k])<<" @ "<<i;
			k++;
		}
	}

	// overwrite put
	for (int i=0; i<num; i++)
		ASSERT_TRUE(block.put(idx[i], &data1[i])==BT_OVERWRITE);
	
	// read back
	k = 0;
	for (int i=block.getLowerBound(); i < block.getUpperBound(); i++) {
		if (idx[k] > i) {
			ASSERT_TRUE(block.get(i, &datum)==BT_OK);
			ASSERT_TRUE(BtreeBlock::IS_DEFAULT(datum))<<" @ "<<i;
		}
		else {
			ASSERT_TRUE(block.get(i, &datum)==BT_OK);
			ASSERT_DOUBLE_EQ(datum, data1[k])<<" @ "<<i;
			k++;
		}
	}
}

TEST(BtreeBlock, DenseLeaf_Delete)
{
	PageImage image;
	PageHandle ph;
	ph.image = &image;
	ph.pid = 0;

	BtreeDLeafBlock block(&ph, 0);
	Key_t endsBy = block.getCapacity();

	// delete non-existent entries
	ASSERT_EQ(block.del(0), BT_OK);
	ASSERT_EQ(block.del(endsBy-1), BT_OK);
	ASSERT_EQ(block.getSize(), 0);

	// put and get back
	Datum_t x = 1.0, y;
	ASSERT_EQ(block.put(1, &x), BT_OK);
	ASSERT_EQ(block.getSize(), 1);
	ASSERT_EQ(block.get(1, &y), BT_OK);
	ASSERT_DOUBLE_EQ(y, x);

	// delete and get back
	ASSERT_EQ(block.del(1), BT_OK);
	ASSERT_EQ(block.getSize(), 0);
	ASSERT_EQ(block.get(1, &y), BT_OK);
	ASSERT_TRUE(BtreeBlock::IS_DEFAULT(y));
}


/************************************************
 * Sparse leaf block tests.
 */


TEST(BtreeBlock, SparseLeaf_CreateEmpty)
{
	PageImage image;
	PageHandle ph;
	ph.image = &image;
	ph.pid = 0;

	BtreeSLeafBlock block(&ph, 0, 1000);
    // Key_t endsBy = block.getCapacity();
	
	ASSERT_EQ(block.getSize(), 0);
	ASSERT_EQ(block.getLowerBound(), 0);
	ASSERT_EQ(block.getUpperBound(), 1000);
	
	Datum_t datum;

	for (int i=block.getLowerBound(); i < block.getUpperBound(); i++) {
		ASSERT_EQ(block.get(i, &datum), BT_NOT_FOUND);
	}
}

TEST(BtreeBlock, SparseLeaf_PutGet)
{
	PageImage image;
	PageHandle ph;
	ph.image = &image;
	ph.pid = 0;

	Key_t endsBy = 500;

	BtreeSLeafBlock block(&ph, 0, endsBy);

	// 5 pairs of key,datum to be inserted
	const int num = 5;
	Key_t idx[] = {0, 11, 25, 67, endsBy-1};
	Datum_t data[] = {1.0, 2.0, 3.0, 4.0, 5.0};
	Datum_t data1[num];
	for (int i=0; i<num; i++)
		data1[i] = data[i] + 1.0;

	// non-overwrite put
	for (int i=0; i<num; i++)
		ASSERT_TRUE(block.put(idx[i], data+i)==BT_OK);
	
	// read back
	Datum_t datum;
	int k = 0;
	for (int i=block.getLowerBound(); i < block.getUpperBound(); i++) {
		if (idx[k] > i) {
			ASSERT_TRUE(block.get(i, &datum)==BT_NOT_FOUND);
		}
		else {
			ASSERT_TRUE(block.get(i, &datum)==BT_OK);
			ASSERT_DOUBLE_EQ(datum, data[k])<<" @ "<<i;
			k++;
		}
	}

	// overwrite put
	for (int i=0; i<5; i++)
		ASSERT_TRUE(block.put(idx[i], data1+i)==BT_OVERWRITE);
	
	// read back
	k = 0;
	for (int i=block.getLowerBound(); i < block.getUpperBound(); i++) {
		if (idx[k] > i) {
			ASSERT_TRUE(block.get(i, &datum)==BT_NOT_FOUND);
		}
		else {
			ASSERT_TRUE(block.get(i, &datum)==BT_OK);
			ASSERT_DOUBLE_EQ(datum, data1[k])<<" @ "<<i;
			k++;
		}
	}
}


TEST(BtreeBlock, SparseLeaf_Overflow)
{
	PageImage image;
	PageHandle ph;
	ph.image = &image;
	ph.pid = 0;

	Key_t endsBy = 500;

	// range is sparseCap + 3
	BtreeSLeafBlock block(&ph, 0, endsBy);
    key_t cap = block.getCapacity();

	// fill the block to make it look like:
	// oxoxxx...xo (o: empty slots)
	Datum_t x = 1.0;
	ASSERT_TRUE(block.put(1, &x) == BT_OK);
	for (int i=3; i<cap+2; i++) {
		x = i;
		ASSERT_TRUE(block.put(i, &x)==BT_OK);
	}

	Key_t key1 = cap + 2;
	Key_t key2 = 0;
	Key_t key3 = 2;

	// insert to the end to cause overflow
	ASSERT_TRUE(block.put(key1, &x)==BT_OVERFLOW);

	BtreeSparseBlock::OverflowEntry &e = block.overflowEntries[0];
	ASSERT_TRUE(e.idx == cap)<<e.idx<<key1;
	ASSERT_DOUBLE_EQ(*((Datum_t*)e.data), x);

	// clear last insert
	block.getSize()--;

	// insert to the front to cause overflow
	ASSERT_TRUE(block.put(key2, &x)==BT_OVERFLOW);

	// BtreeBlock::OverflowEntry &e = block.overflowEntries[0];
	ASSERT_TRUE(e.idx == 0);
	ASSERT_DOUBLE_EQ(*((Datum_t*)e.data), x);

	// clear last insert
	block.getSize()--;

	// insert in the middle to cause overflow
	ASSERT_TRUE(block.put(key3, &x)==BT_OVERFLOW);

	// BtreeBlock::OverflowEntry &e = block.overflowEntries[0];
	ASSERT_TRUE(e.idx == 1);
	ASSERT_DOUBLE_EQ(*((Datum_t*)e.data), x);
	
}

TEST(BtreeBlock, SparseLeaf_Delete)
{
	PageImage image;
	PageHandle ph;
	ph.image = &image;
	ph.pid = 0;

	Key_t endsBy = 500;

	// range is sparseCap 
	BtreeSLeafBlock block(&ph, 0, endsBy);
    Key_t cap = block.getCapacity();

	Datum_t x = 1.0, y;
	// put and get back
	ASSERT_TRUE(block.put(1, &x) == BT_OK);
	ASSERT_EQ(block.getSize(), 1);
	block.get(1, &y);
	ASSERT_DOUBLE_EQ(y, x);
	// delete
	ASSERT_TRUE(block.del(1) == BT_OK);
	ASSERT_EQ(block.getSize(), 0);
	// read back
	ASSERT_TRUE(block.get(1, &y) == BT_NOT_FOUND);

	// delete non-existent value
	ASSERT_TRUE(block.del(2) == BT_NOT_FOUND);
	ASSERT_TRUE(block.get(2, &y) == BT_NOT_FOUND);
}

/************************************************
 * Internal block tests.
 */

TEST(BtreeBlock, Internal_CreateEmpty)
{
	PageImage image;
	PageHandle ph;
	ph.image = &image;
	ph.pid = 0;

	Key_t endsBy = 800;

	BtreeIntBlock block(&ph, 0, endsBy);
	Key_t cap = block.getCapacity();
	Key_t lower = block.getLowerBound();
	Key_t upper = block.getUpperBound();
	
	ASSERT_EQ(block.getSize(), 0);
	ASSERT_EQ(block.getLowerBound(), 0);
	ASSERT_EQ(block.getUpperBound(), endsBy);
	
	Datum_t datum;

	for (int i=lower; i < upper; i++) {
		ASSERT_EQ(block.get(0, &datum), BT_NOT_FOUND);
	}
}

TEST(BtreeBlock, Internal_PutGet)
{
	PageImage image;
	PageHandle ph;
	ph.image = &image;
	ph.pid = 0;

	Key_t endsBy = 800;

	BtreeIntBlock block(&ph, 0, endsBy);
	Key_t lower = block.getLowerBound();
	Key_t upper = block.getUpperBound();

	// 5 pairs of key,datum to be inserted
	const int num = 5;
	Key_t idx[] = {0, 11, 25, 67, endsBy-1};
	PID_t data[] = {1, 2, 3, 4, 5};
	int order[] = {3, 0 ,4, 1, 2}; // order in which the entries are inserted
	PID_t data1[num];
	for (int i=0; i<num; i++)
		data1[i] = data[i] + 1;

	// non-overwrite put
	for (int i=0; i<5; i++)
		ASSERT_EQ(block.put(idx[order[i]], data+order[i]), BT_OK);
	
	// read back
	PID_t datum;
	int k = 0;
	for (int i=lower; i < upper; i++) {
		if (idx[k] > i) {
			ASSERT_EQ(block.get(i, &datum), BT_NOT_FOUND);
		}
		else {
			ASSERT_EQ(block.get(i, &datum), BT_OK);
			ASSERT_EQ(datum, data[k])<<" @ "<<i;
			k++;
		}
	}

	// overwriting child pointers does not make practical sense, but is
	// tested anyway.
	for (int i=0; i<5; i++)
		ASSERT_EQ(block.put(idx[i], data1+i), BT_OVERWRITE);
	
	// read back
	k = 0;
	for (int i=lower; i < upper; i++) {
		if (idx[k] > i) {
			ASSERT_EQ(block.get(i, &datum), BT_NOT_FOUND);
		}
		else {
			ASSERT_EQ(block.get(i, &datum), BT_OK);
			ASSERT_EQ(datum, data1[k])<<" @ "<<i;
			k++;
		}
	}
}


TEST(BtreeBlock, Internal_Overflow)
{
	PageImage image;
	PageHandle ph;
	ph.image = &image;
	ph.pid = 0;

	Key_t endsBy = 1000;

	// range is sparseCap + 3
	BtreeIntBlock block(&ph, 0, endsBy);
	Key_t cap = block.getCapacity();

	// fill the block to make it look like:
	// oxoxxx...xo (o: empty slots)
	PID_t x = 1;
	ASSERT_TRUE(block.put(1, &x) == BT_OK);
	for (int i=3; i<cap+2; i++) {
		x = i;
		ASSERT_TRUE(block.put(i, &x)==BT_OK);
	}

	Key_t key1 = cap + 2;
	Key_t key2 = 0;
	Key_t key3 = 2;

	// insert to the end to cause overflow
	ASSERT_TRUE(block.put(key1, &x)==BT_OVERFLOW);

	BtreeSparseBlock::OverflowEntry &e = block.overflowEntries[0];
	ASSERT_TRUE(e.idx == cap)<<e.idx<<key1;
	ASSERT_EQ(*((PID_t*)e.data), x);

	// clear last insert
	block.getSize()--;

	// insert to the front to cause overflow
	ASSERT_TRUE(block.put(key2, &x)==BT_OVERFLOW);

	// BtreeBlock::OverflowEntry &e = block.overflowEntries[0];
	ASSERT_TRUE(e.idx == 0);
	ASSERT_EQ(*((PID_t*)e.data), x);

	// clear last insert
	block.getSize()--;

	// insert in the middle to cause overflow
	ASSERT_TRUE(block.put(key3, &x)==BT_OVERFLOW);

	// BtreeBlock::OverflowEntry &e = block.overflowEntries[0];
	ASSERT_TRUE(e.idx == 1);
	ASSERT_EQ(*((PID_t*)e.data), x);
	
}

TEST(BtreeBlock, Internal_Delete)
{
	PageImage image;
	PageHandle ph;
	ph.image = &image;
	ph.pid = 0;

	Key_t endsBy = 1000;

	BtreeIntBlock block(&ph, 0, endsBy);

	PID_t x = 1, y;
	// put and get back
	ASSERT_TRUE(block.put(1, &x) == BT_OK);
	ASSERT_EQ(block.getSize(), 1);
	block.get(1, &y);
	ASSERT_EQ(y, x);
	// delete
	ASSERT_TRUE(block.del(1) == BT_OK);
	ASSERT_EQ(block.getSize(), 0);
	// read back; should be the default value
	ASSERT_TRUE(block.get(1, &y) == BT_NOT_FOUND);

	// delete non-existent value
	ASSERT_TRUE(block.del(2) == BT_NOT_FOUND);
	ASSERT_TRUE(block.get(2, &y) == BT_NOT_FOUND);
}
