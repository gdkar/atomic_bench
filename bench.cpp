#include "microbench/microbench.h"
#include "function.h"

#include <functional>
#include <memory>
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
    std::cout << "std::function<int(int)> construction" << std::endl << 
		moodycamel::microbench_stats(
			[&]() { fn = [=](int a)mutable{ i+=j * a;return i; };r32=fn(r32);},
			100, /* iterations per test run */
			100000 /* number of test runs */
		) << std::endl << std::endl;   // ms -> ns

    auto cfn = fn;
    std::cout << "std::function<int(int)> copy takes" << std::endl << 
		moodycamel::microbench_stats(
			[&]() { cfn = fn;r32=cfn(r32);},
			100, /* iterations per test run */
			100000 /* number of test runs */
		)   // ms -> ns
	<<std::endl<<std::endl;
    auto ffn = func::function<int(int)>{};
    std::cout << "ffn::function<int(int)> construction: " << std::endl << 
		moodycamel::microbench_stats(
			[&]() { ffn = [=](int a)mutable{ i+=j * a;return i; };r32=ffn(r32);},
			100, /* iterations per test run */
			100000 /* number of test runs */
		)   // ms -> ns
	<<std::endl<<std::endl;

    auto cffn = ffn;
    std::cout << "func::function<int(int)> copy: " <<  std::endl << 
		moodycamel::microbench_stats(
			[&]() { cffn = ffn;r32=cffn(r32);},
			100, /* iterations per test run */
			100000 /* number of test runs */
		)   // ms -> ns
	<<std::endl<<std::endl;
    std::cout << "functor construction\n" << std::endl << 
		moodycamel::microbench_stats(
			[&]() { auto sfn = sfunc{r32};r32=sfn(r32);},
			100, /* iterations per test run */
			100000 /* number of test runs */
		)   // ms -> ns
        <<std::endl<<std::endl
	;
    auto sfn = sfunc{};
    std::cout << "functor copy \n" << std::endl <<
		moodycamel::microbench_stats(
			[&]() { auto _sfn = sfn;r32 =_sfn(r32);},
			100, /* iterations per test run */
			100000 /* number of test runs */
		)   // ms -> ns
	<<std::endl<<std::endl;
    std::cout << "lambda construction\n" <<
		moodycamel::microbench_stats(
			[&]() { auto _lambda = [=](int a)mutable{ i+=j*a;return i; };r32=_lambda(r32);},
			100, /* iterations per test run */
			100000 /* number of test runs */
		)   // ms -> ns
	<<std::endl<<std::endl;

    std::cout<<"lambda copy:" << std::endl<<
		moodycamel::microbench_stats(
			[&]() { auto _lambda = lambda;r32=_lambda(r32);},
			100, /* iterations per test run */
			100000 /* number of test runs */
		)   // ms -> ns
	<<std::endl<<std::endl;
    std::cout<<"std::function<int(int)> invocation takes %F clocks on average\n"<<std::endl<<
		moodycamel::microbench_stats(
			[&]() { r32 = fn(r32);},
			100, /* iterations per test run */
			100000 /* number of test runs */
		)   // ms -> ns
	<<std::endl<<std::endl;
	printf(" func::function<int(int)> invocation takes %F clocks on average\n",
		moodycamel::microbench(
			[&]() { r32 = ffn(r32);},
			100, /* iterations per test run */
			100000 /* number of test runs */
		)   // ms -> ns
	);
	printf(" functor invocation takes %F clocks on average\n",
		moodycamel::microbench(
			[&]() { r32 = sfn(r32);},
			100, /* iterations per test run */
			100000 /* number of test runs */
		)   // ms -> ns
	);
	printf(" lambda invocation takes %F clocks on average\n",
		moodycamel::microbench(
			[&]() { r32 = lambda(r32);},
			100, /* iterations per test run */
			100000 /* number of test runs */
		)   // ms -> ns
	);
	printf(" func invocation takes %F clocks on average\n",
		moodycamel::microbench(
			[&]() { r32 = cfunc(r32);},
			100, /* iterations per test run */
			100000 /* number of test runs */
		)   // ms -> ns
	);
	printf(" func ptr invocation takes %F clocks on average\n",
		moodycamel::microbench(
			[&]() { r32 = fptr(r32);},
			100, /* iterations per test run */
			100000 /* number of test runs */
		)   // ms -> ns
	);
	printf(" mutable func invocation takes %F clocks on average\n",
		moodycamel::microbench(
			[&]() { r32 = mfunc(r32);},
			100, /* iterations per test run */
			1000000 /* number of test runs */
		)   // ms -> ns
	);
    fptr = mfunc;
	printf(" mutable func ptr invocation takes %F clocks on average\n",
		moodycamel::microbench(
			[&]() { r32 = fptr(r32);},
			100, /* iterations per test run */
			1000000 /* number of test runs */
		)   // ms -> ns
	);
	printf(" gettime_ns() takes %F clocks on average\n",
		moodycamel::microbench(
			[&]() { r64 = gettime_ns();},
			100, /* iterations per test run */
			100000 /* number of test runs */
		)   // ms -> ns
	);
	printf(" gettime_tsc() takes %F clocks on average\n",
		moodycamel::microbench(
			[&]() { r64 = gettime_tsc();},
			100, /* iterations per test run */
			100000 /* number of test runs */
		)   // ms -> ns
	);
	printf(" gettime_chrono() takes %F clocks on average\n",
		moodycamel::microbench(
			[&]() { r64 = gettime_chrono();},
			100, /* iterations per test run */
			100000 /* number of test runs */
		)   // ms -> ns
	);
    printf("sizeof(lambda) == %zu, sizeof(fn) == %zu, sizeof(fptr) == %zu, sizeof(func) == %zu, sizeof(mfunc) == %zu, sizeof(cfn) == %zu\n",sizeof(lambda), sizeof(fn),sizeof(fptr),sizeof(&cfunc),sizeof(&mfunc), sizeof(cfn));
	return 0;
}
