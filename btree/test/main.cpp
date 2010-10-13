//#include <apr_pools.h>
#include <gtest/gtest.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
//    apr_initialize();
	srand(time(NULL));
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
