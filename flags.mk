#CXXFLAGS += -DPROFILING
#CXXFLAGS += -DPROFILE_BUFMAN
CXXFLAGS += -DUSE_BATCH_BUFFER

vpath %.h lib/SuiteSparse/CHOLMOD/Include

OS = $(shell uname -s)

debug ?= 1
ifeq ($(debug), 1)
	CXXFLAGS += -g -DDEBUG
	SOFLAG += -g
	OBJDIR = obj_debug
else
	CXXFLAGS += -g -O3 -DNDEBUG
	SOFLAG += -g -O3
	OBJDIR = obj
endif

# check if dtrace is present
ifneq ($(shell dtrace -v),)
	CXXFLAGS += -DDTRACE_SDT
endif

# check OS
ifeq ($(OS), SunOS)
	CXX = /usr/local/bin/g++
	#CXX = /opt/csw/gcc4/bin/g++
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
		LDFLAGS += /usr/lib64/atlas/libcblas.so
	endif
endif

LDFLAGS += $(addprefix $(RPATH_FLAG), $(LD_RUN_PATH))

$(OBJDIR)/%.o:%.cpp
	@mkdir -p `dirname $@`
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(OBJDIR)/%.dd:%.cpp
	@mkdir -p `dirname $@`
	@$(CXX) $(CXXFLAGS) $(DEPFLAGS) $< | sed -e "s@^\(.*\)\.o[ \t]*:@$(OBJDIR)/$(shell dirname $<)/\1.o :@" > /tmp/dd
	@mv /tmp/dd $@
