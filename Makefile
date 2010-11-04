OS = $(shell uname -s)
DIRS := common lower directly_mapped btree array
DTRACE = dtrace
CXXFLAGS += -I/usr/local/include -I. $(patsubst %,-I%,$(DIRS))

include flags.mk

SRC =
TARGET = libriot_store.so

include $(patsubst %, %/module.mk,$(DIRS))

OBJ := $(patsubst %.cpp,%.o,$(filter %.cpp,$(SRC))) \
	$(patsubst %.c,%.o,$(filter %.c,$(SRC)))
DEPS := $(OBJ:.o=.dd)
SO_OBJ = $(OBJ)
DTRACE_SRC := riot.dtrace
DTRACE_OBJ := riot.o

ifeq ($(OS),SunOS)
	SO_OBJ += $(DTRACE_OBJ)
endif

all: $(TARGET)

-include $(DEPS)

tags: $(OBJ)
	ctags -R

libriot_store.a: $(OBJ)
	ar rcs $@ $^

riot.h: $(DTRACE_SRC)
	$(DTRACE) -h -o $@ -s $^

$(TARGET): $(SO_OBJ) 
	$(CXX) $(CXXFLAGS) -shared -o $@ $^ $(LDFLAGS)

$(DTRACE_OBJ): $(DTRACE_SRC) $(OBJ)
	$(DTRACE) -G -32 -o $@ -s $^

clean:
	rm -f $(OBJ) $(DTRACE_OBJ)
	rm -f $(DEPS)
	rm -f $(TARGET)
