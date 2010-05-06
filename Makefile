DIRS := common lower directly_mapped btree array
CXX = g++
CXXFLAGS += -Wall -g -fPIC $(patsubst %,-I%,$(DIRS))
CXXFLAGS += -DUSE_BATCH_BUFFER

LDFLAGS += -lgtest `pkg-config --libs apr-1`
LDFLAGS += `pkg-config --libs gsl`

OS = `uname -s`
ifeq ($(OS),SunOS)
	LDFLAGS += -R/usr/apr/1.3/lib
endif

LIBS =
SRC =

include $(patsubst %, %/module.mk,$(DIRS))

OBJ := $(patsubst %.cpp,%.o,$(filter %.cpp,$(SRC))) \
	$(patsubst %.c,%.o,$(filter %.c,$(SRC)))


all:libriot_store.so install

install: libriot_store.so
	@install $< $(HOME)/lib

libriot_store.a: $(OBJ)
	ar rcs $@ $^

libriot_store.so: $(OBJ)
	#ar rcs $@ $^
	$(CXX) -shared $(LDFLAGS) -o $@ $^

include $(OBJ:.o=.d)

%.d:%.cpp
#$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $< | sed "s/$*.o/& $@/g" > $@'
	./depend.sh `dirname $*` $(CXXFLAGS) $< > $@

clean:
	rm -f $(OBJ)
	rm -f $(OBJ:.o=.d)
	rm -f libriot_store.a
