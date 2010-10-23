DIRS := common lower directly_mapped btree array
CXX = g++
DTRACE = dtrace
CXXFLAGS += -Wall -fPIC -I. $(patsubst %,-I%,$(DIRS))

include flags.mk

#LDFLAGS += `pkg-config --libs apr-1 gsl`

SRC =
TARGET = libriot_store-1.so

include $(patsubst %, %/module.mk,$(DIRS))

OBJ := $(patsubst %.cpp,%.o,$(filter %.cpp,$(SRC))) \
	$(patsubst %.c,%.o,$(filter %.c,$(SRC)))
SO_OBJ = $(OBJ)
DTRACE_SRC := riot.dtrace
DTRACE_OBJ := riot.o

OS = $(shell uname -s)
ifeq ($(OS),SunOS)
	SO_OBJ += $(DTRACE_OBJ)
endif

all: $(TARGET)

tags: $(OBJ)
	ctags -R

libriot_store.a: $(OBJ)
	ar rcs $@ $^

riot.h: $(DTRACE_SRC)
	$(DTRACE) -h -o $@ -s $^

$(TARGET): $(SO_OBJ) 
	$(CXX) -shared $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(DTRACE_OBJ): $(DTRACE_SRC) $(OBJ)
	$(DTRACE) -G -32 -o $@ -s $^

test1: $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^ btree/test/main.cpp btree/test/TestBlock.cpp -L/usr/local/lib -lgtest `pkg-config --libs gsl`

include $(OBJ:.o=.dd)

%.dd:%.cpp
	#$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $< | sed "s/$*.o/& $@/g" > $@'
	./depend.sh `dirname $*` $(CXXFLAGS) $< > $@

clean:
	rm -f $(OBJ) $(DTRACE_OBJ)
	rm -f $(OBJ:.o=.dd)
	rm -f $(TARGET)
