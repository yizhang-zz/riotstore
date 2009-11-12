CXX = g++
CXXFLAGS = -g -std=c++0x

.PHONY: all clean lower

objects := $(patsubst %.cpp, %.o, $(wildcard *.cpp))

.c.o:
	$(CXX) $(CXXFLAGS) -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

all: lower BitmapPagedFile
	@echo "testing"
	./test

%.d: %.cpp
	$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $< | sed "s/$*.o/& $@/g" > $@'

include $(objects:.o=.d)

lower:
	cd .. ; $(MAKE)

BitmapPagedFile: TestBitmapPagedFile.o
	$(CXX) -o test $< ../BitmapPagedFile.o

bf: TestBufferManager.o
	$(CXX) -o testbf $< ../BitmapPagedFile.o ../LRUPageReplacer.o
	./testbf

clean : 
	rm -f *.o
	rm -f *.out
