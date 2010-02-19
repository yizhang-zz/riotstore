DIRS := common lower directly_mapped btree
CXX = g++
CXXFLAGS += -g $(patsubst %,-I%,$(DIRS))
# `pkg-config --cflags-only-I apr-1`
LDFLAGS += -lgtest `pkg-config --libs apr-1`

LIBS =
SRC =

include $(patsubst %, %/module.mk,$(DIRS))

OBJ := $(patsubst %.cpp,%.o,$(filter %.cpp,$(SRC))) \
	$(patsubst %.c,%.o,$(filter %.c,$(SRC)))


all:riot_store.a

riot_store.a: $(OBJ)
	ar rcs $@ $^

include $(OBJ:.o=.d)

%.d:%.cpp
#$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $< | sed "s/$*.o/& $@/g" > $@'
	./depend.sh `dirname $*` $(CXXFLAGS) $< > $@

clean:
	@rm -f $(OBJ)
	@rm -f $(OBJ:.o=.d)
	@rm -f riot_store.a
