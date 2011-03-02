LD_RUN_PATH = ../..
include ../../flags.mk
CXXFLAGS += -I../.. -I../../common -I../../btree -I../../lower -I../../directly_mapped
LDFLAGS += $(shell pkg-config --libs gsl)
LDFLAGS += -L../.. -lriot_store -ldb

.PHONY: all clean lower

SRC :=  write.cpp gen.cpp read.cpp write1.cpp dump.cpp gentr.cpp toworkload.cpp testequal.cpp
OBJ := $(SRC:.cpp=.o)
EXE := $(SRC:.cpp=)

all: $(EXE)

-include $(OBJ:.o=.dd)

%:%.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean : 
	rm -f $(EXE)
	rm -f $(OBJ)
	rm -f $(OBJ:.o=.dd)
