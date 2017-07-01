ifeq ($(OS),Windows_NT)
	EXT = .exe
	PLATLNOPTS =
else
	EXT =
	PLATLNOPTS = -lm -lstdc++ -static-libstdc++ #-lrt -lm -ldl
endif

ifneq (,$(findstring clang,$(CXX)))
  INCLUDES=-I/usr/local/include/c++/v1/ -I/opt/rh/devtoolset-3/root/usr/include/c++/4.9.2/
  LDFLAGS+= -L/usr/local/lib -lc++
endif
CC=/opt/cross/x86_64-linux-musl/bin/x86_64-linux-musl-gcc
CXX=/opt/cross/x86_64-linux-musl/bin/x86_64-linux-musl-g++

OPTFLAGS+= -g -ggdb -O3 -Ofast -ffast-math -march=native
CXXFLAGS+= -std=gnu++14 -static -pthread -fPIC
CFLAGS+=   -std=gnu11 -pthread
SRCS := bench.cpp microbench/systemtime.cpp

default: bench$(EXT)
run: bench$(EXT)
	./bench$(EXT)
clean:
	rm ./bench$(EXT)
bench: bench.cpp microbench/microbench.h microbench/systemtime.h microbench/systemtime.cpp atomic_bitops.hpp makefile divisor.hpp function.hpp ilog2.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTFLAGS) $(CPPFLAGS) -DNDEBUG -O3 $(SRCS) -o bench $(PLATLNOPTS) $(LDFLAGS)


