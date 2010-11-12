CXX = g++
CXXFLAGS += -Wall -g 
#CXXFLAGS += `pkg-config --cflags-only-I apr-1`
# enables profiling
CXXFLAGS += -DPROFILE_BUFMAN
CXXFLAGS += -DPROFILING

#CXXFLAGS += -DUSE_BATCH_BUFFER

LDFLAGS += `pkg-config --libs gsl`
include ../flags.mk

LIBS =
SRC =

all:elemops mat_write

%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< ../libriot_store.so $(LDFLAGS)

clean:
	@rm -f elemops
