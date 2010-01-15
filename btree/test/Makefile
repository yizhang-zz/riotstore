CXX = g++
CXXFLAGS = -g 
LDFLAGS = -lgtest

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

test: $(objects) $(par_objects) ../../common/common.o ../../lower/BitmapPagedFile.o ../../lower/LRUPageReplacer.o 
	echo $(par_objects)
	$(CXX) $(LDFLAGS) -o test $^ 


clean : 
	rm -f *.o
	rm -f *.out
	rm -f *.d
