DIRS := common lower directly_mapped btree array
CXX = g++
CXXFLAGS += -Wall -g -fPIC $(patsubst %,-I%,$(DIRS))
# `pkg-config --cflags-only-I apr-1`
LDFLAGS += -lgtest `pkg-config --libs apr-1`

LIBS =
SRC =

include $(patsubst %, %/module.mk,$(DIRS))

OBJ := $(patsubst %.cpp,%.o,$(filter %.cpp,$(SRC))) \
	$(patsubst %.c,%.o,$(filter %.c,$(SRC)))


all:libriot_store.a

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
