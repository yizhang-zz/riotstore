#include <gtest/gtest.h>
#include "../BtreeBlock.h"

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
		ASSERT_EQ(block.get(0, &datum), 0);
		ASSERT_TRUE(ISNA(datum));
	}
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
			ASSERT_TRUE(ISNA(datum))<<" @ "<<i;
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
			ASSERT_TRUE(ISNA(datum))<<" @ "<<i;
		}
		else {
			ASSERT_TRUE(block.get(i, &datum)==BT_OK);
			ASSERT_DOUBLE_EQ(datum, data1[k])<<" @ "<<i;
			k++;
		}
	}
}

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
		ASSERT_EQ(block.get(0, &datum), 0);
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
	BtreeBlock block(&ph, 0, endsBy+2, true, false);

	// fill the block to make it look like:
	// o x o x x x ... x o (o: empty slots)
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
