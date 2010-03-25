#include <apr_pools.h>
#include <gtest/gtest.h>

int main(int argc, char **argv) {
    apr_initialize();
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
