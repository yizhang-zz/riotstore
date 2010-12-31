LD_RUN_PATH = ../..
include ../../flags.mk
CXXFLAGS += -I../.. -I../../common -I../../btree -I../../lower
LDFLAGS += $(shell pkg-config --libs gsl)
LDFLAGS += -L../.. -lriot_store

.PHONY: all clean lower

SRC :=  write.cpp gen.cpp read.cpp
OBJ := $(patsubst %.cpp, %.o, $(SRC))
EXE := $(SRC:.cpp=)

all: $(EXE)

-include $(OBJ:.o=.dd)

%:%.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean : 
	rm -f $(EXE)
	rm -f $(OBJ)
	rm -f $(OBJ:.o=.dd)
