CXXFLAGS += -g
#CXXFLAGS += -O2
#CXXFLAGS += -DPROFILING
CXXFLAGS += -DDISABLE_DENSE_LEAF
#CXXFLAGS += -DUSE_BATCH_BUFFER

OS = $(shell uname -s)
ifeq ($(OS), SunOS)
	RPATH_FLAG := -R
else
	RPATH_FLAG := -Wl,-rpath,
endif

LDFLAGS += $(addprefix $(RPATH_FLAG), $(LD_RUN_PATH))
