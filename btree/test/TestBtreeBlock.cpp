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

	Key_t endsBy = BtreeBlock::denseCap;

	BtreeBlock block(&ph, 0, endsBy, true, true);
	
	ASSERT_EQ(block.nEntries, 0);
	ASSERT_EQ(block.lower, 0);
	ASSERT_EQ(block.upper, endsBy);
	
	Datum_t datum;

	for (int i=block.lower; i < block.upper; i++) {
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

	Key_t endsBy = BtreeBlock::denseCap;

	BtreeBlock block(&ph, 0, endsBy, true, true);
	Datum_t datum = 1.0;
	block.put(1, &datum);
	block.put(46, &datum);
	block.syncHeader();

	// init another block with existing data
	BtreeBlock block1(&ph, 0, endsBy);
	ASSERT_EQ(block1.nEntries, 2);
	ASSERT_EQ(block1.lower, 0);
	ASSERT_EQ(block1.upper, endsBy);
	ASSERT_EQ(block1.isLeaf, block.isLeaf);
	ASSERT_EQ(block1.isDense, block.isDense);
}

TEST(BtreeBlock, DenseLeaf_PutGet)
{
	PageImage image;
	PageHandle ph;
	ph.image = &image;
	ph.pid = 0;

	Key_t endsBy = BtreeBlock::denseCap;

	BtreeBlock block(&ph, 0, endsBy, true, true);

	// 5 pairs of key,datum to be inserted
	const int num = 5;
	Key_t idx[] = {0, 11, 25, 67, endsBy-1};
	Datum_t data[] = {1.0, 2.0, 3.0, 4.0, 5.0};
	Datum_t data1[num];
	for (int i=0; i<num; i++)
		data1[i] = data[i] + 1.0;

	// non-overwrite put
	for (int i=0; i<5; i++)
		ASSERT_TRUE(block.put(idx[i], data+i)==BT_OK);
	
	// read back
	Datum_t datum;
	int k = 0;
	for (int i=block.lower; i < block.upper; i++) {
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
	for (int i=0; i<5; i++)
		ASSERT_TRUE(block.put(idx[i], data1+i)==BT_OVERWRITE);
	
	// read back
	k = 0;
	for (int i=block.lower; i < block.upper; i++) {
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

	Key_t endsBy = BtreeBlock::denseCap;
	BtreeBlock block(&ph, 0, endsBy, true, true);

	// delete non-existent entries
	ASSERT_EQ(block.del(0), BT_OK);
	ASSERT_EQ(block.del(endsBy-1), BT_OK);
	ASSERT_EQ(block.nEntries, 0);

	// put and get back
	Datum_t x = 1.0, y;
	ASSERT_EQ(block.put(1, &x), BT_OK);
	ASSERT_EQ(block.nEntries, 1);
	ASSERT_EQ(block.get(1, &y), BT_OK);
	ASSERT_DOUBLE_EQ(y, x);

	// delete and get back
	ASSERT_EQ(block.del(1), BT_OK);
	ASSERT_EQ(block.nEntries, 0);
	ASSERT_EQ(block.get(1, &y), BT_OK);
	ASSERT_DOUBLE_EQ(y, BtreeBlock::defaultValue);
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

	Key_t endsBy = BtreeBlock::sparseCap;

	BtreeBlock block(&ph, 0, endsBy, true, false);
	
	ASSERT_EQ(block.nEntries, 0);
	ASSERT_EQ(block.lower, 0);
	ASSERT_EQ(block.upper, endsBy);
	
	Datum_t datum;

	for (int i=block.lower; i < block.upper; i++) {
		ASSERT_EQ(block.get(i, &datum), BT_OK);
		ASSERT_DOUBLE_EQ(datum, 0.0);
	}
}

TEST(BtreeBlock, SparseLeaf_PutGet)
{
	PageImage image;
	PageHandle ph;
	ph.image = &image;
	ph.pid = 0;

	Key_t endsBy = BtreeBlock::sparseCap;

	BtreeBlock block(&ph, 0, endsBy, true, false);

	// 5 pairs of key,datum to be inserted
	const int num = 5;
	Key_t idx[] = {0, 11, 25, 67, endsBy-1};
	Datum_t data[] = {1.0, 2.0, 3.0, 4.0, 5.0};
	Datum_t data1[num];
	for (int i=0; i<num; i++)
		data1[i] = data[i] + 1.0;

	// non-overwrite put
	for (int i=0; i<5; i++)
		ASSERT_TRUE(block.put(idx[i], data+i)==BT_OK);
	
	// read back
	Datum_t datum;
	int k = 0;
	for (int i=block.lower; i < block.upper; i++) {
		if (idx[k] > i) {
			ASSERT_TRUE(block.get(i, &datum)==BT_OK);
			ASSERT_DOUBLE_EQ(datum, 0.0)<<" @ "<<i;
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
	for (int i=block.lower; i < block.upper; i++) {
		if (idx[k] > i) {
			ASSERT_TRUE(block.get(i, &datum)==BT_OK);
			ASSERT_DOUBLE_EQ(datum, 0.0)<<" @ "<<i;
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

	Key_t endsBy = BtreeBlock::sparseCap;

	// range is sparseCap + 3
	BtreeBlock block(&ph, 0, endsBy+3, true, false);

	// fill the block to make it look like:
	// oxoxxx...xo (o: empty slots)
	Datum_t x = 1.0;
	ASSERT_TRUE(block.put(1, &x) == BT_OK);
	for (int i=3; i<endsBy+2; i++) {
		x = i;
		ASSERT_TRUE(block.put(i, &x)==BT_OK);
	}

	Key_t key1 = endsBy + 2;
	Key_t key2 = 0;
	Key_t key3 = 2;

	// insert to the end to cause overflow
	ASSERT_TRUE(block.put(key1, &x)==BT_OVERFLOW);

	BtreeBlock::OverflowEntry &e = block.overflowEntries[0];
	ASSERT_TRUE(e.idx == endsBy)<<e.idx<<key1;
	ASSERT_DOUBLE_EQ(*((Datum_t*)e.data), x);

	// clear last insert
	block.nEntries--;

	// insert to the front to cause overflow
	ASSERT_TRUE(block.put(key2, &x)==BT_OVERFLOW);

	// BtreeBlock::OverflowEntry &e = block.overflowEntries[0];
	ASSERT_TRUE(e.idx == 0);
	ASSERT_DOUBLE_EQ(*((Datum_t*)e.data), x);

	// clear last insert
	block.nEntries--;

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

	Key_t endsBy = BtreeBlock::sparseCap;

	// range is sparseCap 
	BtreeBlock block(&ph, 0, endsBy, true, false);

	Datum_t x = 1.0, y;
	// put and get back
	ASSERT_TRUE(block.put(1, &x) == BT_OK);
	ASSERT_EQ(block.nEntries, 1);
	block.get(1, &y);
	ASSERT_DOUBLE_EQ(y, x);
	// delete
	ASSERT_TRUE(block.del(1) == BT_OK);
	ASSERT_EQ(block.nEntries, 0);
	// read back; should be the default value
	ASSERT_TRUE(block.get(1, &y) == BT_OK);
	ASSERT_DOUBLE_EQ(y, BtreeBlock::defaultValue);

	// delete non-existent value
	ASSERT_TRUE(block.del(2) == BT_OK);
	ASSERT_TRUE(block.get(2, &y) == BT_OK);
	ASSERT_DOUBLE_EQ(y, BtreeBlock::defaultValue);
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

	Key_t endsBy = BtreeBlock::internalCap * 2;

	BtreeBlock block(&ph, 0, endsBy, false, false);
	
	ASSERT_EQ(block.nEntries, 0);
	ASSERT_EQ(block.lower, 0);
	ASSERT_EQ(block.upper, endsBy);
	
	Datum_t datum;

	for (int i=block.lower; i < block.upper; i++) {
		ASSERT_EQ(block.get(0, &datum), BT_NOT_FOUND);
	}
}

TEST(BtreeBlock, Internal_PutGet)
{
	PageImage image;
	PageHandle ph;
	ph.image = &image;
	ph.pid = 0;

	Key_t endsBy = BtreeBlock::internalCap * 2;

	BtreeBlock block(&ph, 0, endsBy, false, false);

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
		ASSERT_TRUE(block.put(idx[order[i]], data+order[i])==BT_OK);
	
	// read back
	PID_t datum;
	int k = 0;
	for (int i=block.lower; i < block.upper; i++) {
		if (idx[k] > i) {
			ASSERT_TRUE(block.get(i, &datum)==BT_NOT_FOUND);
		}
		else {
			ASSERT_TRUE(block.get(i, &datum)==BT_OK);
			ASSERT_EQ(datum, data[k])<<" @ "<<i;
			k++;
		}
	}

	// overwriting child pointers does not make practical sense, but is
	// tested anyway.
	for (int i=0; i<5; i++)
		ASSERT_TRUE(block.put(idx[i], data1+i)==BT_OVERWRITE);
	
	// read back
	k = 0;
	for (int i=block.lower; i < block.upper; i++) {
		if (idx[k] > i) {
			ASSERT_TRUE(block.get(i, &datum)==BT_NOT_FOUND);
		}
		else {
			ASSERT_TRUE(block.get(i, &datum)==BT_OK);
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

	Key_t endsBy = BtreeBlock::internalCap;

	// range is sparseCap + 3
	BtreeBlock block(&ph, 0, endsBy+3, false, false);

	// fill the block to make it look like:
	// oxoxxx...xo (o: empty slots)
	PID_t x = 1;
	ASSERT_TRUE(block.put(1, &x) == BT_OK);
	for (int i=3; i<endsBy+2; i++) {
		x = i;
		ASSERT_TRUE(block.put(i, &x)==BT_OK);
	}

	Key_t key1 = endsBy + 2;
	Key_t key2 = 0;
	Key_t key3 = 2;

	// insert to the end to cause overflow
	ASSERT_TRUE(block.put(key1, &x)==BT_OVERFLOW);

	BtreeBlock::OverflowEntry &e = block.overflowEntries[0];
	ASSERT_TRUE(e.idx == endsBy)<<e.idx<<key1;
	ASSERT_EQ(*((PID_t*)e.data), x);

	// clear last insert
	block.nEntries--;

	// insert to the front to cause overflow
	ASSERT_TRUE(block.put(key2, &x)==BT_OVERFLOW);

	// BtreeBlock::OverflowEntry &e = block.overflowEntries[0];
	ASSERT_TRUE(e.idx == 0);
	ASSERT_EQ(*((PID_t*)e.data), x);

	// clear last insert
	block.nEntries--;

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

	Key_t endsBy = BtreeBlock::internalCap;

	BtreeBlock block(&ph, 0, endsBy, false, false);

	PID_t x = 1, y;
	// put and get back
	ASSERT_TRUE(block.put(1, &x) == BT_OK);
	ASSERT_EQ(block.nEntries, 1);
	block.get(1, &y);
	ASSERT_EQ(y, x);
	// delete
	ASSERT_TRUE(block.del(1) == BT_OK);
	ASSERT_EQ(block.nEntries, 0);
	// read back; should be the default value
	ASSERT_TRUE(block.get(1, &y) == BT_NOT_FOUND);

	// delete non-existent value
	ASSERT_TRUE(block.del(2) == BT_NOT_FOUND);
	ASSERT_TRUE(block.get(2, &y) == BT_NOT_FOUND);
}
