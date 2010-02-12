CXX = g++
CXXFLAGS = -g 
LDFLAGS = -lgtest

.PHONY: all clean lower

objects := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
#objects := TestFile.o TestBufferManager.o main.o
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

test: $(objects) $(par_objects) ../../common/common.o ../BitmapPagedFile.o ../LRUPageReplacer.o 
	echo $(par_objects)
	$(CXX) $(LDFLAGS) -o test $^ 

#BitmapPagedFile: ../../common/common.o ../BitmapPagedFile.o TestBitmapPagedFile.o
	#$(CXX) -o test $^

#bf: TestBufferManager.o
	#$(CXX) -o testbf $< ../BitmapPagedFile.o ../LRUPageReplacer.o
	#./testbf

clean : 
	rm -f *.o
	rm -f *.out
	rm -f *.d
