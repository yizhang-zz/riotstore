CXX = g++
include ../../flags.mk
CXXFLAGS += -I/usr/local/include -I../..
LDFLAGS += $(shell pkg-config --libs gsl)
LDFLAGS += -L../.. -R../.. -lriot_store -L/usr/local/lib -lgtest

.PHONY: all clean lower

SRC := TestBatch.cpp main.cpp #TestBlock.cpp TestSplit.cpp TestBtree.cpp 
OBJ := $(patsubst %.cpp, %.o, $(SRC))
DEPS := $(OBJ:.o=.dd)
#par_objects := $(patsubst %.cpp, %.o, $(wildcard ../*.cpp))
#lower_objects := $(wildcard ../../lower/*.o)


all: test

include $(DEPS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

%.dd: %.cpp
	$(CXX) -M $(CXXFLAGS) $< | sed "s/$*.o/& $@/g" > $@

test: $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)


clean : 
	rm -f $(OBJ)
	rm -f $(DEPS)
	rm -f test

