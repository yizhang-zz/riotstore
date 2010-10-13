CXX = g++
CXXFLAGS = -g -I/usr/local/include 
#`pkg-config --cflags-only-I apr-1`
#LDFLAGS = `pkg-config --libs apr-1`
#LDFLAGS += `pkg-config --libs `
LDFLAGS += -L../.. -R../.. -lriot_store -L/usr/local/lib -lgtest

.PHONY: all clean lower

SRC := main.cpp TestBlock.cpp TestSplit.cpp
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

