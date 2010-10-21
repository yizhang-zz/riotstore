CXX = g++
CXXFLAGS = -I/usr/local/include -I../..
LDFLAGS = $(shell pkg-config --libs gsl)
LDFLAGS += -L../.. -R../.. -lriot_store-1 -L/usr/local/lib -lgtest
include ../../flags.mk

.PHONY: all clean lower

SRC := TestBatch.cpp main.cpp TestBlock.cpp TestSplit.cpp TestBtree.cpp 
objects := $(patsubst %.cpp, %.o, $(SRC))
#par_objects := $(patsubst %.cpp, %.o, $(wildcard ../*.cpp))
#lower_objects := $(wildcard ../../lower/*.o)


all: test

include $(objects:.o=.d)

.c.o:
	$(CXX) $(CXXFLAGS) -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

%.d: %.cpp
	$(CXX) -M $(CXXFLAGS) $< | sed "s/$*.o/& $@/g" > $@

test: $(objects)
	$(CXX) -o $@ $^ $(LDFLAGS)


clean : 
	rm -f *.o
	rm -f *.out
	rm -f *.d
	rm -f test

