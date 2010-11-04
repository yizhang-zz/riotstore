CXXFLAGS += -I/usr/local/include -I../..
LDFLAGS += $(shell pkg-config --libs gsl)
LDFLAGS += -L../.. -lriot_store -L/usr/local/lib -lgtest
LD_RUN_PATH = ../..

include ../../flags.mk

.PHONY: all clean lower

SRC := TestBatch.cpp main.cpp #TestBlock.cpp TestSplit.cpp TestBtree.cpp 
OBJ := $(patsubst %.cpp, %.o, $(SRC))
DEPS := $(OBJ:.o=.dd)
#par_objects := $(patsubst %.cpp, %.o, $(wildcard ../*.cpp))
#lower_objects := $(wildcard ../../lower/*.o)


all: test

-include $(DEPS)

test: $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)


clean : 
	rm -f $(OBJ)
	rm -f $(DEPS)
	rm -f test

