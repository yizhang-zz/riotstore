DIRS := common lower directly_mapped btree 
CXX = g++
CXXFLAGS += -Wall -g -fPIC $(patsubst %,-I%,$(DIRS)) -DPROFILING
#CXXFLAGS += -DUSE_BATCH_BUFFER

#LDFLAGS += `pkg-config --libs apr-1 gsl`

OS = $(shell uname -s)
ifeq ($(OS),SunOS)
#LDFLAGS += -R/usr/apr/1.3/lib
endif

LIBS =
SRC =

include $(patsubst %, %/module.mk,$(DIRS))

OBJ := $(patsubst %.cpp,%.o,$(filter %.cpp,$(SRC))) \
	$(patsubst %.c,%.o,$(filter %.c,$(SRC)))


all: libriot_store.so

tags: $(OBJ)
	ctags -R

install: libriot_store.so
	@cp $< $(HOME)/lib/

libriot_store.a: $(OBJ)
	ar rcs $@ $^

libriot_store.so: $(OBJ)
	$(CXX) -shared $(LDFLAGS) -o $@ $^

test1: $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^ btree/test/main.cpp btree/test/TestBlock.cpp -L/usr/local/lib -lgtest
include $(OBJ:.o=.d)

%.d:%.cpp
#$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $< | sed "s/$*.o/& $@/g" > $@'
	./depend.sh `dirname $*` $(CXXFLAGS) $< > $@

clean:
	rm -f $(OBJ)
	rm -f $(OBJ:.o=.d)
	rm -f libriot_store.a
	rm -f libriot_store.so
