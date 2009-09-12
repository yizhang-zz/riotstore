# source files: data.c block.cpp iterator.cpp simple.cpp
# header files: data.h block.h iterator.h simple.h

CCFLAGS = -g
CC = g++
CXX = g++
CXXFLAGS = -g

DATA_O = ../bin/data.o
BLOCK_O = ../bin/block.o
ITERATOR_O = ../bin/iterator.o
UTIL_O = ../bin/util.o
TEST_BLOCK_O = ../bin/test_block.o
TEST_DATA_O = ../bin/test_data.o
TEST_BLOCK_OUT = ../bin/test_block.out

MAINDEPENDENCIES = block.cpp data.c iterator.cpp
MAINOBJECTS = ${DATA_O} ${BLOCK_O} ${ITERATOR_O}

objects := $(patsubst %.c,%.o, $(wildcard *.c)) $(patsubst %.cpp, %.o, $(wildcard *.cpp))

.c.o:
	$(CXX) $(CXXFLAGS) -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

all: $(objects)
	@echo "test"

test_block : test/test_block.cpp ${MAINDEPENDENCIES} util.cpp
	${CC} ${CCFLAGS} -c test/test_block.cpp -o ${TEST_BLOCK_O}
	${CC} ${CCFLAGS} ${MAINOBJECTS} ${UTIL_O} ${TEST_BLOCK_O} -o test_block.out

test_data : test/test_data.cpp data.c util.cpp
	${CC} ${CCFLAGS} -c test/test_data.cpp -o ${TEST_DATA_O}
	${CC} ${CCFLAGS} ${TEST_DATA_O} ${DATA_O} ${UTIL_O} -o test_data.out

simple : simple.cpp simple.h ${MAINDEPENDENCIES}
	${CC} ${CCFLAGS} -c simple.cpp

block : block.h ${MAINDEPENDENCIES}
	${CC} ${CCFLAGS} -c block.cpp -o ${BLOCK_O}

data : data.c data.h
	${CC} ${CCFLAGS} -c data.c -o ${DATA_O}

iterator : iterator.cpp iterator.h data.c
	${CC} ${CCFLAGS} -c iterator.cpp -o ${ITERATOR_O}

util : util.cpp util.h
	${CC} ${CCFLAGS} -c util.cpp -o ${UTIL_O}

clean : 
	rm -f ../bin/*.o
	rm -f ../bin/*.out
