LDFLAGS += -L/usr/local/lib -lgtest -L../.. -lriot_store
LDFLAGS += $(shell pkg-config --libs gsl)
LD_RUN_PATH = ../..

.PHONY: all clean 

objects := TestDMA.o main.o

all: test
	./test

include ../../flags.mk
include $(objects:.o=.dd)


test: $(objects) 
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) 

clean : 
	rm -f *.o
	rm -f *.out
	rm -f *.dd
	rm -f test
