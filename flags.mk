#CXXFLAGS += -DPROFILING
#CXXFLAGS += -DPROFILE_BUFMAN
CXXFLAGS += -DUSE_BATCH_BUFFER

vpath %.h lib/SuiteSparse/CHOLMOD/Include

OS = $(shell uname -s)

debug ?= 1
ifeq ($(debug), 1)
	CXXFLAGS += -g -DDEBUG
	SOFLAG += -g
else
	CXXFLAGS += -O3 -DNDEBUG
	SOFLAG += -O3
endif

ifeq ($(OS), SunOS)
	CXX = g++
	CXXFLAGS += -m64 -msse3 -fPIC -Wall
	DEPFLAGS = -MM -MG
	SOFLAG += -m64 -msse3 -shared 
	LDFLAGS += 
	AR = ar rcs
	#CXX = CC
	#CXXFLAGS += -xarch=sse3 -KPIC -library=stlport4 #-instances=extern
	#DEPFLAGS = -xM1
	#SOFLAG += -G -xarch=sse3 #-instances=extern
	#LDFLAGS += -dalign -library=sunperf -library=stlport4
	#AR = CC -xar -o
	RPATH_FLAG := -R
else
	CXX = g++
	CXXFLAGS += -fPIC -Wall
	DEPFLAGS = -MM -MG
	SOFLAG += -shared
	RPATH_FLAG := -Wl,-rpath,
	AR = ar rcs
	ifeq ($(OS), Darwin)
		LDFLAGS += -framework vecLib
	else
		LDFLAGS += -lcblas -llapack
	endif
endif

LDFLAGS += $(addprefix $(RPATH_FLAG), $(LD_RUN_PATH))

%.o:%.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

%.dd:%.cpp
	@$(CXX) $(CXXFLAGS) $(DEPFLAGS) $< | sed -e "s@^\(.*\)\.o[ \t]*:@$(shell dirname $<)/\1.o :@" > /tmp/dd
	@mv /tmp/dd $@
