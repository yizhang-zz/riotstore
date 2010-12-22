CXXFLAGS += -I../..
CXXFLAGS += -I../../lib/SuiteSparse/CHOLMOD/Include
CXXFLAGS += -I../../lib/SuiteSparse/UFconfig
LDFLAGS += $(shell pkg-config --libs-only-L gsl) -lgsl
LDFLAGS += -framework vecLib
LDFLAGS += -pthread
LDFLAGS += -L../.. -lriot_store -L/usr/local/lib -lgtest 
LDFLAGS += -L../../lib/SuiteSparse/CHOLMOD/Lib -lcholmod
LD_RUN_PATH = ../..

SRC = main.cpp TestBlockBased.cpp TestRowMajor.cpp 
SRC += TestBatch.cpp
SRC += TestArith.cpp
OBJ := $(patsubst %.cpp, %.o, $(SRC))
DEPS := $(OBJ:.o=.dd)

all: test

.PHONY: all clean

include ../../flags.mk
include $(DEPS)

test: $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

clean : 
	rm -f $(OBJ)
	rm -f $(DEPS)
	rm -f test

