#SRC += $(wildcard btree/*.cpp)
SRC += btree/BtreeSparseBlock.cpp btree/BtreeDenseLeafBlock.cpp \
	   btree/BtreeConfig.cpp btree/BtreeBlock.cpp \
	   btree/Splitter.cpp
#btree/BtreeBlock.cpp
CXXFLAGS += `pkg-config --cflags-only-I gsl`
