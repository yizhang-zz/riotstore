#include <gtest/gtest.h>

using namespace std;

int main(int argc, char **argv) {
    int seed = time(NULL);
    if (argc > 1) 
        seed = atoi(argv[1]);
    srand(seed);
    cout<<"seed="<<seed<<endl;
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
