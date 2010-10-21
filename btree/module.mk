#SRC += $(wildcard btree/*.cpp)
SRC += \
	btree/BtreeDenseLeafBlock.cpp \
	btree/BtreeSparseBlock.cpp \
	btree/BtreeBlock.cpp \
	btree/Splitter.cpp btree/Btree.cpp \
	btree/BtreeDenseIterator.cpp \
	btree/BatchBufferFWF.cpp
#btree/BtreeBlock.cpp
CXXFLAGS += `pkg-config --cflags-only-I gsl`
