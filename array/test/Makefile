CXX = g++ 
CXXFLAGS = -g `pkg-config --cflags-only-I apr-1`
LDFLAGS = -lgtest `pkg-config --libs apr-1`

.PHONY: all clean lower

objects := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
par_objects := $(patsubst %.cpp, %.o, $(wildcard ../*.cpp))

.c.o:
	$(CXX) $(CXXFLAGS) -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

all: lower test
	./test

%.d: %.cpp
	$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $< | sed "s/$*.o/& $@/g" > $@'

include $(objects:.o=.d)

lower:
	cd .. ; $(MAKE)

test: $(objects) $(par_objects) ../../common/common.o ../../lower/BitmapPagedFile.o ../../lower/LRUPageReplacer.o ../../lower/BufferManager.o ../../directly_mapped/DirectlyMappedArray.o ../../directly_mapped/DenseArrayBlock.o ../../directly_mapped/DMADenseIterator.o ../../directly_mapped/DABIterator.o
	$(CXX) $(LDFLAGS) -o test $^ 


clean : 
	rm -f *.o
	rm -f *.d
	rm test
