
CXX = g++
CXXFLAGS = -g -std=c++0x

objects := $(patsubst %.c,%.o, $(wildcard *.c)) $(patsubst %.cpp, %.o, $(wildcard *.cpp))

all: $(objects)
	@echo "test"

%.d: %.cpp
	$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $< | sed "s/$*.o/& $@/g" > $@'

include $(objects:.o=.d)

#.c.o:
#	$(CXX) $(CXXFLAGS) -c $<

#.cpp.o:
#	$(CXX) $(CXXFLAGS) -c $<

clean : 
	rm -f *.o
	rm -f *.out
	rm -f *.d
