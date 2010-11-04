#include <gtest/gtest.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>

int main(int argc, char **argv) {
	int seed;
	if (argc > 1) {
		seed = atoi(argv[1]);
	}
	else
		seed = time(NULL);
	std::cout<<"seed="<<seed<<std::endl;
	srand(seed);
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
