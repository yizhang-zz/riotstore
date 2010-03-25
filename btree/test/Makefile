CXX = g++
CXXFLAGS = -g `pkg-config --cflags-only-I apr-1`
LDFLAGS = -L/usr/local/lib -lgtest `pkg-config --libs apr-1` -R/usr/apr/1.3/lib

.PHONY: all clean lower

objects := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
par_objects := $(patsubst %.cpp, %.o, $(wildcard ../*.cpp))
lower_objects := $(wildcard ../../lower/*.o)

.c.o:
	$(CXX) $(CXXFLAGS) -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

all: test
	./test
#LD_LIBRARY_PATH=$(HOME)/lib ./test

%.d: %.cpp
	$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $< | sed "s/$*.o/& $@/g" > $@'



test: $(objects) $(par_objects) $(lower_objects) ../../common/common.o
	echo $(par_objects)
	$(CXX) $(LDFLAGS) -o test $^ 


clean : 
	rm -f *.o
	rm -f *.out
	rm -f *.d


include $(objects:.o=.d)
