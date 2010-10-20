CXX = g++
CXXFLAGS = -g `pkg-config --cflags-only-I apr-1`
LDFLAGS = -L/usr/local/lib -lgtest
LDFLAGS += $(shell pkg-config --libs gsl)
LDFLAGS += -L../.. -R../.. -lriot_store

.PHONY: all clean lower

SRC := TestPermute.cpp 
OBJ := $(patsubst %.cpp, %.o, $(SRC))

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

all: test

%.d: %.cpp
	$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $< | sed "s/$*.o/& $@/g" > $@'

include $(objects:.o=.d)

lower:
	cd .. ; $(MAKE)

test: $(OBJ) 
	$(CXX) -o $@ $^ $(LDFLAGS) 

clean : 
	rm -f *.o
	rm -f *.out
	rm -f *.d
