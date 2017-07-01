#include "microbench/microbench.h"
#include "function.hpp"
#include "ilog2.hpp"
#include "atomic_bitops.hpp"
#include "divisor.hpp"
#include <climits>
#include <bitset>
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


inline int64_t gettime_normal(void)
{
    auto spec = timespec{};
    clock_gettime(CLOCK_REALTIME, &spec);
    return 1000000000 * spec.tv_sec + spec.tv_nsec;
}
inline int64_t gettime_tsc(void)
{
    auto auxv = uint32_t{};
    return __rdtscp(&auxv);
}
inline int64_t gettime_chrono(void)
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
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
template<class Func, std::uint64_t iters = 16, std::uint32_t reps = 256>
void do_bench(const std::string &str, Func &&func) {
    std::cout << str << std::endl;
    std::cout << moodycamel::microbench<Func,iters,reps>(std::forward<Func&&>(func),true) 
    << std::endl << std::endl;
}
template<class Func, std::uint64_t iters = 8, std::uint32_t reps = 256>
void do_bench4(const std::string &str,Func &&func) {
    std::cout << str << std::endl;
    std::cout << moodycamel::microbench<Func,iters,reps,4>(std::forward<Func&&>(func),true) 
    << std::endl << std::endl;
}
static int (*fptr)(int a);
int main(int argc, char** argv)
{
    uint64_t seed[2] = { 0u, 0u};
    for(auto &s : seed) {
        s = 0;
        auto t = uint32_t{0};
        for(auto shift : { 0, 32}) {
            for(auto i = 0; i < 1024; i ++ ){
                if(!_rdrand32_step(&t))
                    break;
                for(auto j = 0; j < 64; j++)
                    _mm_pause();
            }
            s |= t << shift;
        }
    }

    auto rand_xr = [=]() mutable {
        constexpr auto rotl = [](auto x, auto k){return (x<<k)|(x>>(std::numeric_limits<decltype(x)>::digits-k));};
        const auto s0 = seed[0];
        auto s1       = seed[1];
        const auto re = s0 + s1;
        s1 ^= s0;
        seed[0] = rotl(s0,55) ^ s1 ^ (s1<<14);
        seed[1] = rotl(s1,36);
        return re;
    };
    struct rand_cache{
        uint64_t seed[2];
        const size_t   m_mask   = 127ul;
        std::array<uint64_t,128ul> m_data{};
        size_t                   m_index{0};
        uint64_t gen_one() {
            constexpr auto rotl = [](auto x, auto k){return (x<<k)|(x>>(std::numeric_limits<decltype(x)>::digits-k));};
            const auto s0 = seed[0];
            auto s1       = seed[1];
            const auto re = s0 + s1;
            s1 ^= s0;
            seed[0] = rotl(s0,55) ^ s1 ^ (s1<<14);
            seed[1] = rotl(s1,36);
            return re;
        }
        rand_cache(uint64_t s0, uint64_t s1):seed{s0,s1} { regen();}
        uint64_t operator() () { return m_data[(++m_index)&m_mask];}
        void regen() { std::generate(m_data.begin(),m_data.end(),[this](){return gen_one();});}
    };
    rand_cache rand_c(rand_xr(),rand_xr());
    fptr = cfunc;
	std::atomic<int> x32(0);
	std::atomic<int> rx32(0);
	std::atomic<int> sx32(0);
	std::atomic<int> tx32(0);
	std::atomic<int> ux32(0);
	int y32 = rand_xr();
	int z32 = rand_xr();
	if (sizeof(x32) != 4)
        exit(1);
	
	std::atomic<long long> x64(0);
	long long y64 = rand_xr();
	long long z64 = rand_xr();
	if (sizeof(x64) != 8)
        exit(2);
	
    std::atomic<__int128> x128(0);
    __int128  y128 = rand_xr();
    __int128 z128 = rand_xr();
    volatile int       r32 = rand_xr();
    volatile int       s32 = rand_xr();
    volatile int       t32 = rand_xr();
    volatile int       u32 = rand_xr();
    volatile long long r64 = rand_xr();
    volatile long long s64 = rand_xr();
    volatile long long t64 = rand_xr();
    volatile long long u64 = rand_xr();
    volatile __int128 r128 = rand_xr();
    volatile __int128 s128 = rand_xr();
    volatile __int128 t128 = rand_xr();
    volatile __int128 u128 = rand_xr();
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
    do_bench(" gettime_normal()", [&]() { r64 = gettime_normal();} );
    do_bench(" gettime_tsc()", [&]() { r64 = gettime_tsc();} );
    do_bench(" gettime_moodycamel()", [&]() { r64 = moodycamel::getSystemTime();} );
    do_bench(" gettime_chrono()", [&]() { r64 = gettime_chrono();} );


    printf("\n");
    auto _32 = r32; auto _64 = r64;
    auto _r32 = r32, _s32 = s32, _t32 = t32, _u32 = u32;
    auto _r64 = r64, _s64 = s64, _t64 = t64, _u64 = u64;
    auto _r128 = r128, _s128 = s128, _t128 = t128, _u128 = u128;
    uint8_t dummy[64];
    for(auto &i : dummy) {
        i = rand_xr();
    }
    auto mask = (sizeof(dummy) * 8u ) - 1u;
    auto wmask = (sizeof(dummy) / sizeof(dummy[0])) - 1u;
    do_bench(" atomic fetch and add, 32 bit.", [&]() { x32.fetch_add(rand_c()); });
    do_bench(" atomic fetch and add, 64 bit.", [&]() { x64.fetch_add(rand_c()); });
    r64 = _r64 + _r32 +_r128;
    s64 = _s64 + _s32 + _s128;
    t64 = _t64 + _t32 + _t128;
    u64 = _u64 + _u32 + _u128;
    do_bench(" atomic fetch and or, 32 bit.", [&]() { x32.fetch_or(rand_c()); });
    do_bench(" atomic fetch and or, 64 bit.", [&]() { x64.fetch_or(rand_c()); });
    do_bench(" atomic fetch and xor, 32 bit.", [&]() { x32.fetch_xor(rand_c()); });
    do_bench(" atomic fetch and xor, 64 bit.", [&]() { x64.fetch_xor(rand_c()); });

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
        _r32 += x32.exchange(_r32,std::memory_order_relaxed);
        _s32 += x32.exchange(_s32,std::memory_order_relaxed);
        _t32 += x32.exchange(_t32,std::memory_order_relaxed);
        _u32 += x32.exchange(_u32,std::memory_order_relaxed);
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
        atomic_test_and_set_bit(&dummy[0], rand_c() & mask);
        atomic_test_and_set_bit(&dummy[0], rand_c() & mask);
        atomic_test_and_set_bit(&dummy[0], rand_c() & mask);
        atomic_test_and_set_bit(&dummy[0], rand_c() & mask);
    });
    do_bench4(" atomic btr. ",[&]() {
        atomic_test_and_clear_bit(&dummy[0], rand_c() & mask);
        atomic_test_and_clear_bit(&dummy[0], rand_c() & mask);
        atomic_test_and_clear_bit(&dummy[0], rand_c() & mask);
        atomic_test_and_clear_bit(&dummy[0], rand_c() & mask);
    });
    do_bench4(" atomic bts. result used",[&]() {
        _r32 += atomic_test_and_set_bit(&dummy[0], rand_c() & mask);
        _s32 += atomic_test_and_set_bit(&dummy[0], rand_c() & mask);
        _t32 += atomic_test_and_set_bit(&dummy[0], rand_c() & mask);
        _u32 += atomic_test_and_set_bit(&dummy[0], rand_c() & mask);
    });
    do_bench4(" atomic btr. result used",[&]() {
        _r32 += atomic_test_and_clear_bit(&dummy[0], rand_c() & mask);
        _s32 += atomic_test_and_clear_bit(&dummy[0], rand_c() & mask);
        _t32 += atomic_test_and_clear_bit(&dummy[0], rand_c() & mask);
        _u32 += atomic_test_and_clear_bit(&dummy[0], rand_c() & mask);
    });
    do_bench4("  bts. ",[&]() {
        test_and_set_bit(&dummy[0], rand_c() & mask);
        test_and_set_bit(&dummy[0], rand_c() & mask);
        test_and_set_bit(&dummy[0], rand_c() & mask);
        test_and_set_bit(&dummy[0], rand_c() & mask);
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
    do_bench4(" ilog2_by_clz(r64) ",[&]() { _r64 += ilog2_by_clz(rand_c());_s64 += ilog2_by_clz(rand_c());_t64 += ilog2_by_clz(rand_c()); _u64 += ilog2_by_clz(rand_c());
    });
    do_bench4(" ilog2_naive(r64) ",[&]() { _r64 += ilog2_naive(rand_c());_s64 += ilog2_naive(rand_c());_t64 += ilog2_naive(rand_c()); _u64 += ilog2_naive(rand_c());
    });
do_bench4(" ilog2_by_cmath_and_cast(r64) ",[&]() { _r64 += ilog2_by_cmath_and_cast(rand_c());_s64 += ilog2_by_cmath_and_cast(rand_c());_t64 += ilog2_by_cmath_and_cast(rand_c()); _u64 += ilog2_by_cmath_and_cast(rand_c());
    });
    do_bench4(" ilog2_by_clz(r32) ",[&]() { _r32 += ilog2_by_clz(rand_c());_s32 += ilog2_by_clz(rand_c());_t32 += ilog2_by_clz(rand_c()); _u32 += ilog2_by_clz(rand_c());
    });
    do_bench4(" ilog2_naive(r32) ",[&]() { _r32 += ilog2_naive(rand_c());_s32 += ilog2_naive(rand_c());_t32 += ilog2_naive(rand_c()); _u32 += ilog2_naive(rand_c());
    });
do_bench4(" ilog2_by_cmath_and_cast(r32) ",[&]() { _r32 += ilog2_by_cmath_and_cast(rand_c());_s32 += ilog2_by_cmath_and_cast(rand_c());_t32 += ilog2_by_cmath_and_cast(rand_c()); _u32 += ilog2_by_cmath_and_cast(rand_c());
    });


    do_bench4(" __builtin_ffsl(r64) ",[&]() { _r64 += __builtin_ffsll(rand_c());_s64 += __builtin_ffsll(rand_c());_t64 += __builtin_ffsll(rand_c()); _u64 += __builtin_ffsll(rand_c());
    });
    do_bench4(" __builtin_ffs(r32) ",[&]() { _r32 += __builtin_ffs(rand_c());_s32 += __builtin_ffs(rand_c());_t32 += __builtin_ffs(rand_c()); _u32 += __builtin_ffs(rand_c());
    });
    do_bench4(" __builtin_clzl(r64) ",[&]() { _r64 += __builtin_clzll(rand_c());_s64 += __builtin_clzll(rand_c());_t64 += __builtin_clzll(rand_c()); _u64 += __builtin_clzll(rand_c());
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

    auto div_d = rand_xr();
    auto div = divisor(div_d);
    std::cerr << "div_d = " << div_d << std::endl;
    do_bench(" rand_c ",[&]() { _r64 ^= rand_c();});
    r64 = _r64 + _r32 +_r128;s64 = _s64 + _s32 + _s128;t64 = _t64 + _t32 + _t128;u64 = _u64 + _u32 + _u128;
    do_bench(" val / divisor ",[&]() { r64 += (rand_c() / div);});
    r64 = _r64 + _r32 +_r128;s64 = _s64 + _s32 + _s128;t64 = _t64 + _t32 + _t128;u64 = _u64 + _u32 + _u128;
    do_bench(" val * divisor ",[&]() { r64 += (rand_c() * div);});
    r64 = _r64 + _r32 +_r128;s64 = _s64 + _s32 + _s128;t64 = _t64 + _t32 + _t128;u64 = _u64 + _u32 + _u128;

    x32 = r32 + s32 + t32 + u32;
    x64 = r64 + s64 + t64 + u64;
    
    {
        std::bitset<std::numeric_limits<uint32_t>::digits> bitset32{};
        do_bench(" bitset32 operator[] + asign ",[&](){ auto r = rand_c();bitset32[r&31] = r & 32;});
        rand_c.regen();
        do_bench(" bitset32 operator[] + flip ",[&](){ auto r = rand_c();bitset32.flip(r&31);});
        rand_c.regen();
        do_bench(" bitset32 operator[] + flip ",[&](){ auto r = rand_c();_r32 += bitset32[r&31];});
    }
    {
        std::bitset<std::numeric_limits<uint64_t>::digits> bitset64{};
        do_bench(" bitset64 operator[] + asign ",[&](){ auto r = rand_c();bitset64[rand_c()&63] = r & 64;});
        do_bench(" bitset64 operator[] + flip ",[&](){ auto r = rand_c();bitset64.flip(rand_c()&63);});
        do_bench(" bitset64 operator[] + read ",[&](){ auto r = rand_c();_r64 += bitset64[rand_c()&63];});
    }
    printf("sizeof(lambda) == %zu, sizeof(fn) == %zu, sizeof(fptr) == %zu, sizeof(func) == %zu, sizeof(mfunc) == %zu, sizeof(x128) == %zu, sizeof(x64) == %zu, sizeof(x32) == %zu\n",sizeof(lambda), sizeof(fn),sizeof(fptr),sizeof(&cfunc),sizeof(&mfunc),sizeof(x128),sizeof(x64),sizeof(x32));
	return 0;
}
