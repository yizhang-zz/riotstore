CXX = g++
CXXFLAGS += -Wall -g 
CXXFLAGS += `pkg-config --cflags-only-I apr-1`
# enables profiling
CXXFLAGS += -DPROFILING

#CXXFLAGS += -DUSE_BATCH_BUFFER

LDFLAGS += `pkg-config --libs apr-1`
#LDFLAGS += `pkg-config --libs gsl`

OS = `uname -s`
ifeq ($(OS),SunOS)
	LDFLAGS += -R/usr/apr/1.3/lib
endif

LIBS =
SRC =

all:elemops

elemops: elemops.cpp
	$(CXX) $(CXXFLAGS) -o test-elemops elemops.cpp ../libriot_store.so

clean:
	rm -f test-elemops
