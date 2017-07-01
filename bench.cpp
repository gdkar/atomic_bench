#include "microbench/microbench.h"
#include "function.h"
#include "atomic_bitops.h"
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

inline uint32_t ghetto_cpu_id()
{
    static THREAD_LOCAL uint32_t id = 1024;
    static THREAD_LOCAL uint32_t n  = 0;
    if(!n) {
       n = 256;
        __rdtscp(&id);
    }else{
        n --;
    }
    return (id & 255u);
}


int main(int argc, char** argv)
{
    std::atomic<int> x32(0);
    std::atomic<int> x32_0(0);
    std::atomic<int> x32_1(0);
    std::atomic<int> x32_2(0);
    std::atomic<int> x32_3(0);
    std::atomic<int> x32_4(0);
    std::atomic<int> x32_5(0);
    std::atomic<int> x32_6(0);
    std::atomic<int> x32_7(0);
	int y32 = rand();
	int y32_0 = rand();
	int y32_1 = rand();
	int y32_2 = rand();
	int y32_3 = rand();
	int y32_4 = rand();
	int y32_5 = rand();
	int y32_6 = rand();
	int y32_7 = rand();
	int z32 = rand();
    static_assert(sizeof(x32) == 4,"sizeof(int) should be 4, goshdarnit.");
    std::atomic<long long> x64(0);
    std::atomic<long long> x64_0(0);
    std::atomic<long long> x64_1(0);
    std::atomic<long long> x64_2(0);
    std::atomic<long long> x64_3(0);
    std::atomic<long long> x64_4(0);
    std::atomic<long long> x64_5(0);
    std::atomic<long long> x64_6(0);
    std::atomic<long long> x64_7(0);
	long long y64 = rand();
	long long y64_0 = rand();
	long long y64_1 = rand();
	long long y64_2 = rand();
	long long y64_3 = rand();
	long long y64_4 = rand();
	long long y64_5 = rand();
	long long y64_6 = rand();
	long long y64_7 = rand();
	long long z64 = rand();
    static_assert(sizeof(x64) == 8,"sizeof(long long) should be 8, goshdarnit.");
	
    std::atomic<__int128> x128(0);
    __int128 y128 = rand();
    __int128 z128 = rand();
    volatile int32_t r32;
    volatile int64_t r64;
    volatile __int128 r128;
	printf("32-bit CAS takes %.8fclk on average\n",
		moodycamel::microbench(
			[&]() { 
                x32_0.compare_exchange_strong(y32_0, z32, std::memory_order_acq_rel, std::memory_order_acquire);
                x32_1.compare_exchange_strong(y32_1, z32, std::memory_order_acq_rel, std::memory_order_acquire);
                x32_2.compare_exchange_strong(y32_2, z32, std::memory_order_acq_rel, std::memory_order_acquire);
                x32_3.compare_exchange_strong(y32_3, z32, std::memory_order_acq_rel, std::memory_order_acquire);
                x32_4.compare_exchange_strong(y32_4, z32, std::memory_order_acq_rel, std::memory_order_acquire);
                x32_5.compare_exchange_strong(y32_5, z32, std::memory_order_acq_rel, std::memory_order_acquire);
                x32_6.compare_exchange_strong(y32_6, z32, std::memory_order_acq_rel, std::memory_order_acquire);
                x32_7.compare_exchange_strong(y32_7, z32, std::memory_order_acq_rel, std::memory_order_acquire);
            },
			1000, /* iterations per test run */
			50000 /* number of test runs */
		)/8.    // ms -> ns
	);
	printf("64-bit CAS takes %.8f clocks on average\n",
		moodycamel::microbench(
			[&]() { 
                x64_0.compare_exchange_strong(y64_0, z64, std::memory_order_acq_rel, std::memory_order_acquire);
                x64_1.compare_exchange_strong(y64_1, z64, std::memory_order_acq_rel, std::memory_order_acquire);
                x64_2.compare_exchange_strong(y64_2, z64, std::memory_order_acq_rel, std::memory_order_acquire);
                x64_3.compare_exchange_strong(y64_3, z64, std::memory_order_acq_rel, std::memory_order_acquire);
                x64_4.compare_exchange_strong(y64_4, z64, std::memory_order_acq_rel, std::memory_order_acquire);
                x64_5.compare_exchange_strong(y64_5, z64, std::memory_order_acq_rel, std::memory_order_acquire);
                x64_6.compare_exchange_strong(y64_6, z64, std::memory_order_acq_rel, std::memory_order_acquire);
                x64_7.compare_exchange_strong(y64_7, z64, std::memory_order_acq_rel, std::memory_order_acquire);
                },
			1000,
			50000
		)/8.
	);
    printf("128-bit CAS takes %.8f clocks on average\n",
		moodycamel::microbench(
			[&]() { x128.compare_exchange_strong(y128, z128, std::memory_order_acq_rel, std::memory_order_acquire); },
			100,
			100000
		)
	);
	printf("32-bit weak CAS takes %.8f clocks on average\n",
		moodycamel::microbench(
			[&]() { x32.compare_exchange_weak(y32, z32, std::memory_order_acq_rel, std::memory_order_acquire); },
			100, /* iterations per test run */
			500000 /* number of test runs */
		)
	);
	
	printf("64-bit weak CAS takes %.8f clocks on average\n",
		moodycamel::microbench(
			[&]() { x64.compare_exchange_weak(y64, z64, std::memory_order_acq_rel, std::memory_order_acquire); },
			100,
			500000
		)
	);
    printf("128-bit weak CAS takes %.8f clocks on average\n",
		moodycamel::microbench(
			[&]() { x128.compare_exchange_weak(y128, z128, std::memory_order_acq_rel, std::memory_order_acquire); },
			100,
			500000
		)
	);
	printf("32-bit FA& takes %.8f clocks on average\n",
		moodycamel::microbench(
			[&]() { r32 = x32.fetch_and(y32, std::memory_order_relaxed); },
			100,
			500000
		)
	);
	printf("64-bit FA& takes %.8f clocks on average\n",
		moodycamel::microbench(
			[&]() { r64 = x64.fetch_and(y64, std::memory_order_relaxed); },
			100,
			500000
		)
	);
	printf("32-bit lock & takes %.8f clocks on average\n",
		moodycamel::microbench(
			[&]() { x32.fetch_and(y32, std::memory_order_relaxed); },
			100,
			500000
		)
	);
    printf("64-bit bts takes %.8f clocks on average\n",
		moodycamel::microbench(
			[&]() { atomic_test_and_set_bit(&x64, y64&63); },
			100,
			500000
		)
	);
    printf("64-bit btr takes %.8f clocks on average\n",
		moodycamel::microbench(
			[&]() { atomic_test_and_clear_bit(&x64, y64&63); },
			100,
			500000
		)
	);
    printf("64-bit __builtin_ffs takes %.8f clocks on average\n",
		moodycamel::microbench(
			[&]() { r64 = __builtin_ffs( y64); },
			100,
			500000
		)
	);
    printf("64-bit bsrq takes %.8f clocks on average\n",
		moodycamel::microbench(
			[&]() { 
                r32 = __builtin_ctzl(y64);
                },
			100,
			500000
		)
	);
	printf("64-bit lock & takes %.8f clocks on average\n",
		moodycamel::microbench(
			[&]() { x64.fetch_and(y64, std::memory_order_relaxed); },
			100,
			500000
		)
	);
	printf("32-bit FAA takes %.8f clocks on average\n",
		moodycamel::microbench(
			[&]() { r32 = x32.fetch_add(y32, std::memory_order_relaxed); },
			100,
			500000
		)
	);
	
	printf("64-bit FAA takes %.8f clocks on average\n",
		moodycamel::microbench(
			[&]() { r64 = x64.fetch_add(y64, std::memory_order_relaxed); },
			100,
			500000
		)
	);
	printf("32-bit xchg takes %.8f clocks on average\n",
		moodycamel::microbench(
			[&]() { r32 = x32.exchange(y32, std::memory_order_relaxed); },
			100,
			50000
		)
	);
	printf("64-bit xchg takes %.8f clocks on average\n",
		moodycamel::microbench(
			[&]() { r64 = x64.exchange(y64, std::memory_order_relaxed); },
			100,
			500000
		)
	);
	printf("128-bit xchg takes %.8f clocks on average\n",
		moodycamel::microbench(
			[&]() { r128 = x128.exchange(y128, std::memory_order_relaxed); },
			100,
			100000
		)
	);
	printf("std::this_thread::get_id() takes %.8f clocks on average\n",
		moodycamel::microbench(
			[&]() { std::this_thread::get_id(); },
			100,
			100000
		)
	);
    uint32_t ghettoCPU = 0;
    printf("ghetto_cpu_id() takes %.8f clocks on average\n",
		moodycamel::microbench(
			[&]() { ghettoCPU += ghetto_cpu_id(); },
			100,
			100000
		)
	);
}
