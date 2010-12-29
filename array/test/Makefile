CXXFLAGS += -I../.. -I/usr/local/include
CXXFLAGS += -I../../lib/SuiteSparse/CHOLMOD/Include
CXXFLAGS += -I../../lib/SuiteSparse/UFconfig
LDFLAGS += $(shell pkg-config --libs-only-L gsl) -lgsl -lgslcblas
LDFLAGS += -L../.. -lriot_store -L/usr/local/lib -lgtest 
LDFLAGS += -L../../lib/SuiteSparse/CHOLMOD/Lib -lcholmod
LD_RUN_PATH = ../..

SRC = main.cpp TestBlockBased.cpp TestRowMajor.cpp 
SRC += TestBatch.cpp TestBatchLoad.cpp
SRC += TestArith.cpp
OBJ := $(SRC:.cpp=.o)
DEP := $(OBJ:.o=.dd)

all: test

.PHONY: all clean

include ../../flags.mk
include $(DEP)

test: $(OBJ)
	$(CXX) -m64 -o $@ $^ $(LDFLAGS)

clean : 
	rm -f $(OBJ)
	rm -f $(DEP)
	rm -f test

