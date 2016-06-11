ifeq ($(OS),Windows_NT)
	EXT = .exe
	PLATLNOPTS =
else
	EXT =
	PLATLNOPTS = -lrt -lm -ldl
endif

ifneq (,$(findstring clang,$(CXX)))
  INCLUDES=-I/usr/local/include/c++/v1/ -I/opt/rh/devtoolset-3/root/usr/include/c++/4.9.2/
  LDFLAGS+= -L/usr/local/lib -lc++
endif
CC?=gcc
CXX?=g++

OPTFLAGS+= -g -ggdb -O3 -Ofast -march=native
CXXFLAGS+= -std=gnu++14 -pthread
CFLAGS+=   -std=gnu11 -pthread
SRCS := bench.cpp microbench/systemtime.cpp

default: bench$(EXT)
run: bench$(EXT)
	./bench$(EXT)
clean:
	rm ./bench$(EXT)
bench: bench.cpp microbench/microbench.h microbench/systemtime.h microbench/systemtime.cpp atomic_bitops.h makefile
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTFLAGS) $(CPPFLAGS) -DNDEBUG -O3 $(SRCS) -o bench $(PLATLNOPTS) $(LDFLAGS)


