LD_RUN_PATH = ../..
include ../../flags.mk
CXXFLAGS += -I../.. -I../../common -I../../btree -I../../lower -I../../array -I../../lib/SuiteSparse/CHOLMOD/Include -I../../lib/SuiteSparse/UFconfig
LDFLAGS += $(shell pkg-config --libs gsl)
LDFLAGS += -L../.. -lriot_store -ldb

.PHONY: all clean lower

SRC :=  write.cpp write_br.cpp
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
