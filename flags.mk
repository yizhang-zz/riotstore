CXXFLAGS += -DPROFILING
CXXFLAGS += -DPROFILE_BUFMAN
#CXXFLAGS += -DDISABLE_DENSE_LEAF
CXXFLAGS += -DUSE_BATCH_BUFFER

vpath %.h lib/SuiteSparse/CHOLMOD/Include

OS = $(shell uname -s)

ifeq ($(OS), SunOS)
	#CXX = g++
	#CXXFLAGS += -g -DDEBUG -fPIC -Wall
	#DEPFLAGS = -MM -MG
	#LDFLAGS += -xlic_lib=sunperf
	CXX = CC
	CXXFLAGS += -g -xarch=native -KPIC -library=stlport4 -instances=extern
	DEPFLAGS = -xM1
	SOFLAG = -G -g -xarch=native -instances=extern
	LDFLAGS += -dalign -library=sunperf -library=stlport4
	RPATH_FLAG := -R
else
	CXX = g++
	CXXFLAGS += -g -DDEBUG -fPIC -Wall
	DEPFLAGS = -MM -MG
	SOFLAG = -shared -g
	RPATH_FLAG := -Wl,-rpath,
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
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $< | sed -e "s@^\(.*\)\.o[ \t]*:@$(shell dirname $<)/\1.o :@" > /tmp/dd
	mv /tmp/dd $@
