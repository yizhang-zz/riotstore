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

mat_read: mat_read.cpp
	$(CXX) $(CXXFLAGS) -o test-mat-read mat_read.cpp ../libriot_store.so

mat_write: mat_write.cpp
	$(CXX) $(CXXFLAGS) -o test-mat-write mat_write.cpp ../libriot_store.so

elemops: elemops.cpp
	$(CXX) $(CXXFLAGS) -o test-elemops elemops.cpp ../libriot_store.so

clean:
	rm -f test-elemops
