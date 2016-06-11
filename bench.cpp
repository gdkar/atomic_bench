#include "microbench/microbench.h"
#include "function.h"
#include "atomic_bitops.h"
#include "divisor.h"

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

inline int64_t gettime_tsc(void)
{
    auto auxv = uint32_t{};
    return __rdtscp(&auxv);
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
template<class Func, std::uint64_t iters = 32, std::uint32_t reps = 4096>
void do_bench(const std::string &str,
	      Func &&func) {
    std::cout << str << std::endl <<
    moodycamel::microbench<Func,iters,reps>(std::forward<Func&&>(func),true) 
    << std::endl << std::endl;
}
template<class Func, std::uint64_t iters = 8, std::uint32_t reps = 4096>
void do_bench4(const std::string &str,
	      Func &&func) {
    std::cout << str << std::endl <<
    moodycamel::microbench<Func,iters,reps,4>(std::forward<Func&&>(func),true) 
    << std::endl << std::endl;
}
static int (*fptr)(int a);
int main(int argc, char** argv)
{
    fptr = cfunc;
	std::atomic<int> x32(0);
	std::atomic<int> rx32(0);
	std::atomic<int> sx32(0);
	std::atomic<int> tx32(0);
	std::atomic<int> ux32(0);
	int y32 = rand();
	int z32 = rand();
	if (sizeof(x32) != 4) exit(1);
	
	std::atomic<long long> x64(0);
	long long y64 = rand();
	long long z64 = rand();
	if (sizeof(x64) != 8) exit(2);
	
    std::atomic<__int128> x128(0);
    __int128  y128 = rand();
    __int128 z128 = rand();
    volatile int       r32 = rand();
    volatile int       s32 = rand();
    volatile int       t32 = rand();
    volatile int       u32 = rand();
    volatile long long r64 = rand();
    volatile long long s64 = rand();
    volatile long long t64 = rand();
    volatile long long u64 = rand();
    volatile __int128 r128 = rand();
    volatile __int128 s128 = rand();
    volatile __int128 t128 = rand();
    volatile __int128 u128 = rand();
    auto fn = std::function<int(int)>{};
    auto i  = 5;
    auto j  = 7;
    auto lambda = [=](int a)mutable{ i+=j* a;return i; };r32=lambda(r32);
    do_bench("functor construction\n",
			[&]() { auto sfn = sfunc{r32};r32=sfn(r32);}
		);   // ms -> ns
    auto sfn = sfunc{};
    do_bench("functor copy \n", [&]() { auto _sfn = sfn;r32 =_sfn(r32);}
		);   // ms -> ns
    do_bench("lambda construction\n", [&]() { auto _lambda = [=](int a)mutable{ i+=j*a;return i; };r32=_lambda(r32);});

    do_bench("lambda copy:", [&]() { auto _lambda = lambda;r32=_lambda(r32);});
    do_bench(" functor invocation", [&]() { r32 = sfn(r32);});
    do_bench(" lambda invocation", [&]() { r32 = lambda(r32);} );
    do_bench(" func invocation", [&]() { r32 = cfunc(r32);} );
    do_bench(" func ptr invocation", [&]() { r32 = fptr(r32);} );
    do_bench(" mutable func invocation", [&]() { r32 = mfunc(r32);} );
    fptr = mfunc;
    do_bench(" mutable func ptr invocation",[&]() { r32 = fptr(r32);});
    do_bench(" gettime_tsc()", [&]() { r64 = gettime_tsc();} );

    printf("\n");
    auto _32 = r32; auto _64 = r64;
    auto _r32 = r32, _s32 = s32, _t32 = t32, _u32 = u32;
    auto _r64 = r64, _s64 = s64, _t64 = t64, _u64 = u64;
    auto _r128 = r128, _s128 = s128, _t128 = t128, _u128 = u128;
    uint8_t dummy[64];
    for(auto &i : dummy) {
        i = rand();
    }
    auto mask = (sizeof(dummy) * 8u ) - 1u;
    auto wmask = (sizeof(dummy) / sizeof(dummy[0])) - 1u;
    do_bench(" atomic fetch and add, 32 bit.", [&]() { x32.fetch_add(_r32); });
    do_bench(" atomic fetch and add, 64 bit.", [&]() { x64.fetch_add(_64); });
    r64 = _r64 + _r32 +_r128;
    s64 = _s64 + _s32 + _s128;
    t64 = _t64 + _t32 + _t128;
    u64 = _u64 + _u32 + _u128;
    do_bench(" atomic fetch and or, 32 bit.", [&]() { x32.fetch_or(_32); });
    do_bench(" atomic fetch and or, 64 bit.", [&]() { x64.fetch_or(_64); });
    do_bench(" atomic fetch and xor, 32 bit.", [&]() { x32.fetch_xor(_32); });
    do_bench(" atomic fetch and xor, 64 bit.", [&]() { x64.fetch_xor(_64); });

    do_bench4(" atomic fetch and add, 32 bit. result used.", [&]() { 
        _r32 = x32.fetch_add(_r32);
        _s32 = x32.fetch_add(_s32);
        _t32 = x32.fetch_add(_t32);
        _u32 = x32.fetch_add(_u32);
        });
    do_bench4(" atomic fetch and add, 64 bit. result used.", [&]() {
        _r64 = x64.fetch_add(_r64);
        _s64 = x64.fetch_add(_s64);
        _t64 = x64.fetch_add(_t64);
        _u64 = x64.fetch_add(_u64);
        });
    do_bench4(" atomic fetch and or, 32 bit. result used.", [&]() {
        _r32 = x32.fetch_or(_r32);
        _s32 = x32.fetch_or(_s32);
        _t32 = x32.fetch_or(_t32);
        _u32 = x32.fetch_or(_u32);
        });
    do_bench4(" atomic fetch and or, 64 bit. result used.", [&]() {
        _r64 = x64.fetch_or(_r64);
        _s64 = x64.fetch_or(_s64);
        _t64 = x64.fetch_or(_t64);
        _u64 = x64.fetch_or(_u64);
        });
    do_bench4(" atomic fetch and xor, 32 bit. result used.", [&]() {
        _r32 = x32.fetch_xor(_r32);
        _s32 = x32.fetch_xor(_s32);
        _t32 = x32.fetch_xor(_t32);
        _u32 = x32.fetch_xor(_u32);
        });
    do_bench4(" atomic fetch and xor, 64 bit. result used.", [&]() {
        _r64 = x64.fetch_xor(_r64);
        _s64 = x64.fetch_xor(_s64);
        _t64 = x64.fetch_xor(_t64);
        _u64 = x64.fetch_xor(_u64);
        });

    do_bench4(" atomic exchange, 32 bit.", [&]() {
        _r32 +=x32.exchange(_r32,std::memory_order_relaxed);
        _s32 +=x32.exchange(_s32,std::memory_order_relaxed);
        _t32 +=x32.exchange(_t32,std::memory_order_relaxed);
        _u32 +=x32.exchange(_u32,std::memory_order_relaxed);
    });
    do_bench4(" atomic exchange, 64 bit.", [&]() {
        _r64 += x64.exchange(_r64,std::memory_order_relaxed);
        _s64 += x64.exchange(_s64,std::memory_order_relaxed);
        _t64 += x64.exchange(_t64,std::memory_order_relaxed);
        _u64 += x64.exchange(_u64,std::memory_order_relaxed);
    });
    do_bench4(" atomic compare exchange, 32 bit.", [&]() {
        x32.compare_exchange_strong(_r32,_r32,std::memory_order_relaxed);
        x32.compare_exchange_strong(_s32,_s32,std::memory_order_relaxed);
        x32.compare_exchange_strong(_t32,_t32,std::memory_order_relaxed);
        x32.compare_exchange_strong(_u32,_u32,std::memory_order_relaxed);
    });
    do_bench4(" atomic compare exchange, 64 bit.", [&]() {
        x64.compare_exchange_strong(_r64,_r64,std::memory_order_relaxed);
        x64.compare_exchange_strong(_s64,_s64,std::memory_order_relaxed);
        x64.compare_exchange_strong(_t64,_t64,std::memory_order_relaxed);
        x64.compare_exchange_strong(_u64,_u64,std::memory_order_relaxed);
    });
    do_bench4(" atomic compare exchange, 128 bit.", [&]() {
        x128.compare_exchange_strong(_r128,_r128,std::memory_order_relaxed);
        x128.compare_exchange_strong(_s128,_s128,std::memory_order_relaxed);
        x128.compare_exchange_strong(_t128,_t128,std::memory_order_relaxed);
        x128.compare_exchange_strong(_u128,_u128,std::memory_order_relaxed);
    });
    do_bench4(" atomic bts. ",[&]() {
        atomic_test_and_set_bit(&dummy[0], _r32 & mask);
        atomic_test_and_set_bit(&dummy[0], _s32 & mask);
        atomic_test_and_set_bit(&dummy[0], _t32 & mask);
        atomic_test_and_set_bit(&dummy[0], _u32 & mask);
    });
    do_bench4(" atomic btr. ",[&]() {
        atomic_test_and_clear_bit(&dummy[0], _r32 & mask);
        atomic_test_and_clear_bit(&dummy[0], _s32 & mask);
        atomic_test_and_clear_bit(&dummy[0], _t32 & mask);
        atomic_test_and_clear_bit(&dummy[0], _u32 & mask);
    });
    do_bench4(" atomic bts. result used",[&]() {
        _r32 += 23 + atomic_test_and_set_bit(&dummy[0], _r32 & mask);
        _s32 += 23 + atomic_test_and_set_bit(&dummy[0], _s32 & mask);
        _t32 += 23 + atomic_test_and_set_bit(&dummy[0], _t32 & mask);
        _u32 += 23 + atomic_test_and_set_bit(&dummy[0], _u32 & mask);
    });
    do_bench4(" atomic btr. result used",[&]() {
        _r32 += atomic_test_and_clear_bit(&dummy[0], _r32 & mask);
        _s32 += atomic_test_and_clear_bit(&dummy[0], _s32 & mask);
        _t32 += atomic_test_and_clear_bit(&dummy[0], _t32 & mask);
        _u32 += atomic_test_and_clear_bit(&dummy[0], _u32 & mask);
    });
    do_bench4("  bts. ",[&]() {
        test_and_set_bit(&dummy[0], _r32 & mask);
        test_and_set_bit(&dummy[0], _s32 & mask);
        test_and_set_bit(&dummy[0], _t32 & mask);
        test_and_set_bit(&dummy[0], _u32 & mask);
    });
    do_bench4("  btr. ",[&]() {
        test_and_clear_bit(&dummy[0], _r32 & mask);
        test_and_clear_bit(&dummy[0], _s32 & mask);
        test_and_clear_bit(&dummy[0], _t32 & mask);
        test_and_clear_bit(&dummy[0], _u32 & mask);
    });
    do_bench4("  bts. result used",[&]() {
        _r32 += test_and_set_bit(&dummy[0], _r32 & mask);
        _s32 += test_and_set_bit(&dummy[0], _s32 & mask);
        _t32 += test_and_set_bit(&dummy[0], _t32 & mask);
        _u32 += test_and_set_bit(&dummy[0], _u32 & mask);
    });
    do_bench4("  btr. result used",[&]() {
        _r32 += test_and_clear_bit(&dummy[0], _r32 & mask);
        _s32 += test_and_clear_bit(&dummy[0], _s32 & mask);
        _t32 += test_and_clear_bit(&dummy[0], _t32 & mask);
        _u32 += test_and_clear_bit(&dummy[0], _u32 & mask);
    });
    do_bench4(" __builtin_ffsl(r64) ",[&]() { _r64 += __builtin_ffsll(_r64);_s64 += __builtin_ffsll(_s64);_t64 += __builtin_ffsll(_t64); _u64 += __builtin_ffsll(_u64);
    });
    do_bench4(" __builtin_ffs(r32) ",[&]() { _r32 += __builtin_ffs(_r32);_s32 += __builtin_ffs(_s32);_t32 += __builtin_ffs(_t32); _u32 += __builtin_ffs(_u32);
    });
    do_bench4(" __builtin_clzl(r64) ",[&]() { _r64 += __builtin_clzll(_r64);_s64 += __builtin_clzll(_s64);_t64 += __builtin_clzll(_t64); _u64 += __builtin_clzll(_u64);
    });
    do_bench4(" __builtin_clz(r32) ",[&]() { r32 = __builtin_clz(r32);s32 = __builtin_clz(s32);t32 = __builtin_clz(t32); u32 = __builtin_clz(u32);
    });
    do_bench4(" __builtin_ctzl(r64) ",[&]() { r64 = __builtin_ctzll(r64);s64 = __builtin_ctzll(s64);t64 = __builtin_ctzll(t64); u64 = __builtin_ctzll(u64);
    });
    do_bench4(" __builtin_ctz(r32) ",[&]() { r32 = __builtin_ctz(r32);s32 = __builtin_ctz(s32);t32 = __builtin_ctz(t32); u32 = __builtin_ctz(u32);
    });
    do_bench4(" __builtin_popcountl(r64) ",[&]() { r64 = __builtin_popcountll(r64);s64 = __builtin_popcountll(s64);t64 = __builtin_popcountll(t64); u64 = __builtin_popcountll(u64);
    });
    do_bench4(" __builtin_popcount(r32) ",[&]() { r32 = __builtin_popcount(r32);s32 = __builtin_popcount(s32);t32 = __builtin_popcount(t32); u32 = __builtin_popcount(u32);
    });
    uint32_t seed[2] = { 0u, 0u};
    for(auto &s : seed) {
        for(auto i = 0; i < 1024; i ++ ){
            if(!_rdrand32_step(&s)) break;
            for(auto j = 0; j < 64; j++) _mm_pause();
        }
    }
    auto rand_xr = [=]() mutable { 
        auto s1 = seed[0];
        const auto s0 = seed[1];
        seed[0] = s0;
        s1 ^= s1 << 23;
        return(seed[1] = s1 ^ s0 ^ ( s1 >> 18) ^ ( s0 >> 5)) + s0;
    };
    auto div_d = (uint64_t(seed[0])) | (uint64_t(seed[1]) << 32);
    auto div = divisor(div_d);
    do_bench(" rand_xr ",[&]() { _r64 ^= rand_xr();});
    do_bench(" val / divisor ",[&]() { r64 = (rand_xr() / div);});
    do_bench(" val * divisor ",[&]() { r64 = (rand_xr() * div);});
    r64 = _r64 + _r32 +_r128;
    s64 = _s64 + _s32 + _s128;
    t64 = _t64 + _t32 + _t128;
    u64 = _u64 + _u32 + _u128;

    x32 = r32 + s32 + t32 + u32;
    x64 = r64 + s64 + t64 + u64;

    printf("sizeof(lambda) == %zu, sizeof(fn) == %zu, sizeof(fptr) == %zu, sizeof(func) == %zu, sizeof(mfunc) == %zu, sizeof(x128) == %zu, sizeof(x64) == %zu, sizeof(x32) == %zu\n",sizeof(lambda), sizeof(fn),sizeof(fptr),sizeof(&cfunc),sizeof(&mfunc),sizeof(x128),sizeof(x64),sizeof(x32));
	return 0;
}
