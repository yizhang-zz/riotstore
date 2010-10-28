CXX = g++
include ../../flags.mk
CXXFLAGS += -Wall -I../.. -I../../common -I../../btree -I../../lower
LDFLAGS += $(shell pkg-config --libs gsl)
LDFLAGS += -L../.. -R../.. -lriot_store

.PHONY: all clean lower

SRC :=  write.cpp gen.cpp
OBJ := $(patsubst %.cpp, %.o, $(SRC))

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

all: write gen

%.dd: %.cpp
	$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $< | sed "s/$*.o/& $@/g" > $@'

include $(OBJ:.o=.dd)

write: write.o 
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) 

gen: gen.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean : 
	rm -f $(OBJ)
	rm -f $(OBJ:.o=.dd)
	rm -f mb
