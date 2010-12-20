LDFLAGS += -lgtest -L../.. -lriot_store
LD_RUN_PATH = ../..
VPATH = ../..
CXXFLAGS += -I../..

.PHONY: all clean 

src := $(wildcard *.cpp)
objects := $(src:.cpp=.o)

all: test
	./test

include ../../flags.mk
include $(objects:.o=.dd)


test: $(objects) 
	$(CXX) -o $@ $^ $(LDFLAGS) 

clean : 
	rm -f *.o
	rm -f *.out
	rm -f *.dd
	rm -f test
