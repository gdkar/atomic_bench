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
