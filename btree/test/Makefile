CXX = g++
CXXFLAGS = -g `pkg-config --cflags-only-I apr-1`
LDFLAGS = `pkg-config --libs apr-1`
LDFLAGS += `pkg-config --libs `
LDFLAGS += -L$(HOME)/lib -lriot_store

OSTYPE = $(shell uname)
ifeq ($(OSTYPE), SunOS)
	LDFLAGS += -L/usr/local/lib -R/usr/apr/1.3/lib
endif

.PHONY: all clean lower

SRC := main.cpp TestSplitter.cpp
objects := $(patsubst %.cpp, %.o, $(SRC))
#par_objects := $(patsubst %.cpp, %.o, $(wildcard ../*.cpp))
#lower_objects := $(wildcard ../../lower/*.o)

.c.o:
	$(CXX) $(CXXFLAGS) -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

all: test
	./test

%.d: %.cpp
	$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $< | sed "s/$*.o/& $@/g" > $@'

test: $(objects)
	$(CXX) $(LDFLAGS) -o test $(objects)


clean : 
	rm -f *.o
	rm -f *.out
	rm -f *.d

include $(objects:.o=.d)
