DIRS := common lower directly_mapped btree array
CXX = g++
CXXFLAGS += -Wall -fPIC -I. $(patsubst %,-I%,$(DIRS))

include flags.mk

#LDFLAGS += `pkg-config --libs apr-1 gsl`

OS = $(shell uname -s)
ifeq ($(OS),SunOS)
#LDFLAGS += -R/usr/apr/1.3/lib
endif

SRC =
TARGET = libriot_store-1.so

include $(patsubst %, %/module.mk,$(DIRS))

OBJ := $(patsubst %.cpp,%.o,$(filter %.cpp,$(SRC))) \
	$(patsubst %.c,%.o,$(filter %.c,$(SRC)))


all: $(TARGET)

tags: $(OBJ)
	ctags -R

libriot_store.a: $(OBJ)
	ar rcs $@ $^

$(TARGET): $(OBJ)
	$(CXX) -shared $(LDFLAGS) -o $@ $^

test1: $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^ btree/test/main.cpp btree/test/TestBlock.cpp -L/usr/local/lib -lgtest `pkg-config --libs gsl`
include $(OBJ:.o=.d)

%.d:%.cpp
#$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $< | sed "s/$*.o/& $@/g" > $@'
	./depend.sh `dirname $*` $(CXXFLAGS) $< > $@

clean:
	rm -f $(OBJ)
	rm -f $(OBJ:.o=.d)
	rm -f $(TARGET)
