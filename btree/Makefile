CXX = g++
CXXFLAGS = -g 
LDFLAGS = -lgtest -lapr-1

.PHONY: all clean lower

objects := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
#par_objects := $(patsubst %.cpp, %.o, $(wildcard ../*.cpp))

.c.o:
	$(CXX) $(CXXFLAGS) -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

all: $(objects)
#LD_LIBRARY_PATH=$(HOME)/lib ./test

%.d: %.cpp
	$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $< | sed "s/$*.o/& $@/g" > $@'



test: $(objects) $(par_objects)
	echo $(par_objects)
	$(CXX) $(LDFLAGS) -o test $^ 


clean : 
	rm -f *.o
	rm -f *.out
	rm -f *.d


include $(objects:.o=.d)
