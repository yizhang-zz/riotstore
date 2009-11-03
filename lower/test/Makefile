
CXX = g++
CXXFLAGS = -g -std=c++0x

objects := $(patsubst %.c,%.o, $(wildcard *.c)) $(patsubst %.cpp, %.o, $(wildcard *.cpp))

.c.o:
	$(CXX) $(CXXFLAGS) -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

all: $(objects) ../BitmapPagedFile.o
	$(CXX) -o test1 $^

clean : 
	rm -f *.o
	rm -f *.out
