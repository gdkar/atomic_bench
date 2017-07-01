CC  :=/opt/cross/x86_64-linux-musl/bin/x86_64-linux-musl-gcc
CXX :=/opt/cross/x86_64-linux-musl/bin/x86_64-linux-musl-g++

CPPFLAGS += -Wall -Wextra -g -ggdb -O3 -pthread
OPTFLAGS += -fomit-frame-pointer -ffast-math -I. -fPIC -DPIC -fno-math-errno \
						-freciprocal-math -fassociative-math  -ftree-vectorize \
						-pthread -ftls-model=initial-exec -mfpmath=sse 

CFLAGS += -std=gnu11 -static $(OPTFLAGS) $(CPPFLAGS)
CXXFLAGS += -std=gnu++14 -static $(OPTFLAGS) $(CPPFLAGS) -Wno-c++11-narrowing 
LDFLAGS:= -lm -ldl -static 



ifeq ($(OS),Windows_NT)
	EXT = .exe
	PLATLNOPTS =
else
	EXT =
	PLATLNOPTS = 
endif
LDFLAGS+= -lrt -pthread -lm -lstdc++ -ldl
CPPFLAGS+= -g -ggdb -O3 -Ofast -march=native -pthread
#INCLUDES+=  -I/home/gdkar/.local/include/asm  -I/home/gdkar/.local/include/c++ -I/home/gdkar/.local/include/c++/x86_64-redhat-linux  -I/home/gdkar/.local/include
CXXFLAGS+= -std=gnu++14
default: bench$(EXT)
run: bench$(EXT)
	./bench$(EXT)

bench: bench.cpp microbench/microbench.h microbench/systemtime.h microbench/systemtime.cpp function.h
	$(CXX) $(CPPFLAGS) $(INCLUDES) $(CXXFLAGS) -DNDEBUG bench.cpp microbench/systemtime.cpp -o bench $(PLATLNOPTS) $(LDFLAGS)
