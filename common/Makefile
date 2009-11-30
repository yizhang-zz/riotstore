
CXX = g++
CXXFLAGS = -g 

objects := $(patsubst %.c,%.o, $(wildcard *.c)) $(patsubst %.cpp, %.o, $(wildcard *.cpp))

.c.o:
	$(CXX) $(CXXFLAGS) -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

all: $(objects)
	@echo "test"

clean : 
	rm -f *.o
	rm -f *.out
