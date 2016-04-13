#include "microbench/microbench.h"
#include "function.h"

#include <functional>
#include <memory>
#include <iostream>
#include <cstdio>
#include <sstream>
#include <fstream>
#include <utility>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <atomic>
#include <thread>
#include <cstdint>
extern "C" {
#include <dlfcn.h>
#include <sys/auxv.h>
};
#ifdef __APPLE__
#include <pthread.h>
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#define THREAD_LOCAL __declspec(thread)
#else
#define THREAD_LOCAL thread_local
#endif
#include <chrono>

extern "C" {
    extern "C" int __vdso_clock_gettime(int,struct timespec*);
};
void *vdso;
struct vdso_load {
    vdso_load(const char *name)
    {
        vdso = reinterpret_cast<void*>(reinterpret_cast<uintptr_t> (getauxval(AT_SYSINFO_EHDR)));
    }
    inline int64_t gettime() const
    {
        timespec ts;
        __vdso_clock_gettime(CLOCK_REALTIME,&ts);
        return (1000000ll*ts.tv_sec + ts.tv_nsec);       
    }
};
vdso_load vdso_clock_gettime{"__vdso_clock_gettime"};

inline int64_t gettime_vdso(void)
{
    return vdso_clock_gettime.gettime();
}
inline int64_t gettime_ns(void)
{
    auto ts = timespec{};
    clock_gettime(CLOCK_REALTIME,&ts);
    return (1000000ll*ts.tv_sec + ts.tv_nsec);
}
inline int64_t gettime_tsc(void)
{
    return __rdtsc();
}
inline int64_t gettime_chrono(void)
{
    auto now = std::chrono::high_resolution_clock::now();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()).count();
    return nanos;
}
uintptr_t ghetto_thread_id()
{
	static THREAD_LOCAL int x;
	return reinterpret_cast<uintptr_t>(&x);
}
static int cfunc(int a)
{
    return a * 5;
}
static int mfunc(int a)
{
    static auto i = 0ll;
    i += a;
    return i;
}
struct sfunc {
    int i = 5;
    int j = 7;
    sfunc(int _i = 5, int _j = 7)
        :i(_i),j(_j)
    {}
    sfunc(const sfunc&)=default;
    sfunc &operator=(const sfunc&) = default;
    virtual int operator() (int a)
    {
        i += j * a;
        return i;
    }

};
template<class Func, int iters = 64, int reps = 1000>
void do_bench(const std::string &str,
	      Func &&func) {
    std::cout << str << std::endl <<
    moodycamel::microbench_stats(func, iters, reps) 
    << std::endl << std::endl;
}
static int (*fptr)(int a);
int main(int argc, char** argv)
{
    fptr = cfunc;
	std::atomic<int> x32(0);
	int y32 = rand();
	int z32 = rand();
	if (sizeof(x32) != 4) exit(1);
	
	std::atomic<long long> x64(0);
	long long y64 = rand();
	long long z64 = rand();
	if (sizeof(x64) != 8) exit(2);
	
    std::atomic<long double> x128(0);
    long double  y128 = rand();
    long double z128 = rand();
    volatile int       r32 = 0;
    volatile long long r64 = 0;
    volatile long double r128 = 0;
    auto fn = std::function<int(int)>{};
    auto i  = 5;
    auto j  = 7;
    auto lambda = [=](int a)mutable{ i+=j* a;return i; };r32=lambda(r32);
    do_bench( "std::function<int(int)> construction",
			[&]() { fn = [=](int a)mutable{ i+=j * a;return i; };r32=fn(r32);});

    auto cfn = fn;
    do_bench("std::function<int(int)> copy takes",
			[&]() { cfn = fn;r32=cfn(r32);}
		);
    auto ffn = func::function<int(int)>{};
    do_bench("ffn::function<int(int)> construction: ",
			[&]() { ffn = [=](int a)mutable{ i+=j * a;return i; };r32=ffn(r32);}
		);   // ms -> ns);

    auto cffn = ffn;
    do_bench("func::function<int(int)> copy: ",
			[&]() { cffn = ffn;r32=cffn(r32);}
		)   // ms -> ns;
	    ;
    do_bench("functor construction\n",
			[&]() { auto sfn = sfunc{r32};r32=sfn(r32);}
		);   // ms -> ns
    auto sfn = sfunc{};
    do_bench("functor copy \n",
			[&]() { auto _sfn = sfn;r32 =_sfn(r32);}
		);   // ms -> ns
    do_bench("lambda construction\n",
			[&]() { auto _lambda = [=](int a)mutable{ i+=j*a;return i; };r32=_lambda(r32);});

    do_bench("lambda copy:",
			[&]() { auto _lambda = lambda;r32=_lambda(r32);});
    do_bench("std::function<int(int)> invocation takes %F clocks on average\n",
			[&]() { r32 = fn(r32);}
		);
    do_bench(" func::function<int(int)> invocation takes %F clocks on average\n",
			[&]() { r32 = ffn(r32);}
		)   // ms -> ns
	    ;
    do_bench(" functor invocation",
			[&]() { r32 = sfn(r32);}
	);
    do_bench(" lambda invocation",
			[&]() { r32 = lambda(r32);}
	);
    do_bench(" func invocation",
			[&]() { r32 = cfunc(r32);}
	);
    do_bench(" func ptr invocation",
			[&]() { r32 = fptr(r32);}
	);
    do_bench(" mutable func invocation",
			[&]() { r32 = mfunc(r32);}
	);
    fptr = mfunc;
    do_bench(" mutable func ptr invocation",[&]() { r32 = fptr(r32);});
    do_bench(" gettime_ns()",[&]() { r64 = gettime_ns();});
    do_bench(" gettime_tsc()",
			[&]() { r64 = gettime_tsc();}
	);
	do_bench(" gettime_chrono()",
			[&]() { r64 = gettime_chrono();}
	);
    printf("sizeof(lambda) == %zu, sizeof(fn) == %zu, sizeof(fptr) == %zu, sizeof(func) == %zu, sizeof(mfunc) == %zu, sizeof(cfn) == %zu\n",sizeof(lambda), sizeof(fn),sizeof(fptr),sizeof(&cfunc),sizeof(&mfunc), sizeof(cfn));
	return 0;
}
