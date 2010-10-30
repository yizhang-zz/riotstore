OS = $(shell uname -s)

ifeq ($(OS), SunOS)
	CC = g++
	CCFLAGS += -g -O2 -Wall -fPIC
	DEPFLAGS = -MM -MG
	#CC = CC
	#CCFLAGS += -g -xO2 -xarch=native -KPIC #-library=stlport4
	#DEPFLAGS = -xM1
	RPATH_FLAG := -R
else
	CC = g++
	CCFLAGS += -g -O2 -Wall -fPIC
	DEPFLAGS = -MM -MG
	RPATH_FLAG := -Wl,-rpath,
endif

CCFLAGS += -DDISABLE_DENSE_LEAF
CCFLAGS += -DUSE_BATCH_BUFFER


LDFLAGS += $(addprefix $(RPATH_FLAG), $(LD_RUN_PATH))

%.o:%.cpp
	$(CC) -c $(CCFLAGS) -o $@ $<

%.dd:%.cpp
	@DIR=$(shell dirname $<) $(CC) $(CCFLAGS) $(DEPFLAGS) $< | sed -e "s@^\(.*\)\.o:@$DIR/\1.dd $DIR/\1.o:@" > /tmp/dd
	@mv /tmp/dd $@
	@#./depend.sh `dirname $*` $< > $@
