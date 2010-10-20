#include "../../common/common.h"
#include "../BufferManager.h"
#include "../PageReplacer.h"
#include "../BitmapPagedFile.h"
#include <gtest/gtest.h>
#include <assert.h>
#include <stdio.h>
#include <iostream>
using namespace std;

TEST(BitmapPagedFile, Allocate)
{
	char file[] = "bpf.bin";
	int num = NUM_BITS_HEADER;
	BitmapPagedFile bpf(file, BitmapPagedFile::CREATE);
	for (PID_t i=0; i<num; ++i) {
		ASSERT_EQ(RC_OK, bpf.allocatePageWithPID(i));
	}
	ASSERT_EQ(RC_OutOfRange, bpf.allocatePageWithPID(num));
	ASSERT_EQ(RC_OutOfRange, bpf.disposePage(num));

	const int count = 100;
	PID_t pids[count];
	kPermute(pids, (PID_t)0, (PID_t)num-1, count);
	for (int i=0; i<count; ++i) {
		ASSERT_EQ(RC_OK, bpf.disposePage(pids[i]));
		ASSERT_EQ(RC_NotAllocated, bpf.disposePage(pids[i]));
	}
	for (int i=0; i<count; ++i) {
		ASSERT_EQ(RC_OK, bpf.allocatePageWithPID(pids[i]));
	}
}
