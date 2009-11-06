CXX = g++
CXXFLAGS = -g -std=c++0x

.PHONY: all clean lower

objects := $(patsubst %.c,%.o, $(wildcard *.c)) $(patsubst %.cpp, %.o, $(wildcard *.cpp))

.c.o:
	$(CXX) $(CXXFLAGS) -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

all: lower BitmapPagedFile
	@echo "testing"
	./test

lower:
	cd .. ; $(MAKE)

BitmapPagedFile: TestBitmapPagedFile.o
	$(CXX) -o test $< ../BitmapPagedFile.o

clean : 
	rm -f *.o
	rm -f *.out
