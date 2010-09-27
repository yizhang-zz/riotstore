SRC += $(wildcard btree/*.cpp)
CXXFLAGS += `pkg-config --cflags-only-I apr-1 gsl`
