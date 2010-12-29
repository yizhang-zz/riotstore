OS = $(shell uname -s)
DIRS := common lower directly_mapped btree array
DTRACE = dtrace
CXXFLAGS += -I. $(patsubst %,-I%,$(DIRS))
CXXFLAGS += -I/usr/local/include
CXXFLAGS += -Ilib/SuiteSparse/CHOLMOD/Include
CXXFLAGS += -Ilib/SuiteSparse/UFconfig
LDFLAGS += -Llib/SuiteSparse/CHOLMOD/Lib -lcholmod
LDFLAGS += $(shell pkg-config --libs-only-L gsl) -lgsl
include $(patsubst %, %/module.mk,$(DIRS))
OBJ := $(patsubst %.cpp,%.o,$(filter %.cpp,$(SRC))) 
DEPS := $(OBJ:.o=.dd)
DTRACE_SRC := riot.dtrace
DTRACE_OBJ := riot.o
SO_OBJ = $(OBJ)
ifeq ($(OS),SunOS)
	SO_OBJ += $(DTRACE_OBJ)
endif
TARGET = libriot_store

include flags.mk

.PHONY: all suitesparse clean

all: $(TARGET).so

suitesparse:
	@cd lib/SuiteSparse ; make 

tags: $(OBJ)
	ctags -R

$(TARGET).a: $(SO_OBJ)
	$(AR) $@ $^

riot.h: $(DTRACE_SRC)
	$(DTRACE) -h -o $@ -s $^

$(TARGET).so: $(SO_OBJ) 
	$(CXX) $(SOFLAG) -o $@ $^ $(LDFLAGS)

$(DTRACE_OBJ): $(DTRACE_SRC) $(OBJ)
	$(DTRACE) -G -64 -o $@ -s $^

-include $(DEPS)

clean:
	rm -f $(OBJ) $(DTRACE_OBJ)
	rm -f $(DEPS)
	rm -f $(TARGET)
