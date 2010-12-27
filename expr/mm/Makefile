SRC = loadmm.cpp
EXE = $(SRC:.cpp=)
CXXFLAGS += -I../.. -I/usr/local/include -I../../lib/SuiteSparse/CHOLMOD/Include -I../../lib/SuiteSparse/UFconfig
LDFLAGS += $(shell pkg-config --libs gsl) 
LDFLAGS += -L../.. -lriot_store

all: $(EXE)

%:%.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

libriot_store.so:
	@cd ../..; $(MAKE)
	@cp ../../libriot_store.so .

include ../../flags.mk

