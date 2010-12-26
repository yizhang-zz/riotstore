LDFLAGS += -L/usr/local/lib -lgtest -L../.. -lriot_store
LDFLAGS += $(shell pkg-config --libs gsl)
LD_RUN_PATH = ../..
VPATH = ../..
CXXFLAGS += -I../.. -I/usr/local/include

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
