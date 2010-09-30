CXX = g++
CXXFLAGS += -Wall -g 
CXXFLAGS += `pkg-config --cflags-only-I apr-1`
# enables profiling
CXXFLAGS += -DPROFILING

#CXXFLAGS += -DUSE_BATCH_BUFFER

LDFLAGS += `pkg-config --libs apr-1 gsl`
#LDFLAGS += `pkg-config --libs gsl`

OS = $(shell uname -s)
ifeq ($(OS),SunOS)
	LDFLAGS += -L/lib 
endif

LIBS =
SRC =

all:elemops

%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< ../libriot_store.so $(LDFLAGS)

clean:
	@rm -f elemops
