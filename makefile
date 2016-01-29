ifeq ($(OS),Windows_NT)
	EXT = .exe
	PLATLNOPTS =
else
	EXT =
	PLATLNOPTS = 
endif
LDFLAGS+= -lrt -pthread -lm 
CPPFLAGS+= -g -ggdb -O3 -Ofast -march=native -pthread
CXXFLAGS+= -std=gnu++11
default: bench$(EXT)
run: bench$(EXT)
	./bench$(EXT)

bench: bench.cpp microbench/microbench.h microbench/systemtime.h microbench/systemtime.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -DNDEBUG bench.cpp microbench/systemtime.cpp -o bench $(PLATLNOPTS) $(LDFLAGS)
