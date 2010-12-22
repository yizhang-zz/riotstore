CXXFLAGS += -DPROFILING
CXXFLAGS += -DPROFILE_BUFMAN
#CXXFLAGS += -DDISABLE_DENSE_LEAF
CXXFLAGS += -DUSE_BATCH_BUFFER

vpath %.h lib/SuiteSparse/CHOLMOD/Include

OS = $(shell uname -s)

ifeq ($(OS), SunOS)
	CXX = g++
	CXXFLAGS += -g -DDEBUG -fPIC -Wall
	DEPFLAGS = -MM -MG
	#CXX = CXX
	#CXXFLAGS += -g -xO2 -xarch=native -KPIC -library=stlport4
	#DEPFLAGS = -xM1
	RPATH_FLAG := -R
else
	CXX = g++
	CXXFLAGS += -g -DDEBUG -fPIC -Wall
	DEPFLAGS = -MM -MG
	RPATH_FLAG := -Wl,-rpath,
endif

LDFLAGS += $(addprefix $(RPATH_FLAG), $(LD_RUN_PATH))

%.o:%.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

%.dd:%.cpp
	@$(CXX) $(CXXFLAGS) $(DEPFLAGS) $< | sed -e "s@^\(.*\)\.o:@$(shell dirname $<)/\1.dd $(shell dirname $<)/\1.o:@" > /tmp/dd
	@mv /tmp/dd $@
