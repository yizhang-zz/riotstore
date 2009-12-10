CXX = g++
CXXFLAGS = -g 

.PHONY: all clean 

objects := $(patsubst %.cpp, %.o, $(wildcard *.cpp))

.c.o:
	$(CXX) $(CXXFLAGS) -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

all: DMA
	@echo "testing"
	./test

%.d: %.cpp
	$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $< | sed "s/$*.o/& $@/g" > $@'

include $(objects:.o=.d)

DMA: TestDMA.o ../../lower/BitmapPagedFile.o ../../lower/LRUPageReplacer.o ../../common/common.o
	$(CXX) -o test $^

clean : 
	rm -f *.o
	rm -f *.d
	rm -f ../*.o
	rm -f ../*.d

