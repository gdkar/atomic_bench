_Pragma("once")

#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <atomic>
#include <x86intrin.h>

typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

struct divisor {
    uint64_t        N{0};
    uint64_t        d{0};
    uint128_t       m{0};
    uint32_t        p{0};
    uint32_t        s{0};
    divisor(const divisor&) = default;
    divisor(divisor&&)      = default;
    divisor& operator=(const divisor&) = default;
    divisor& operator=(divisor&&)      = default;
    divisor(uint64_t _d, uint64_t _N = 64)
    : N(_N)
    , d(_d)
    {
        for(; d >> p && p < N - 1ul; p++) {}
        for(; d >> s && !((d>>s)&1) && s < N - 1ul; s++) {}
        m = ((uint128_t)1 << ( N + p)) / ((uint128_t)(d >> s));
    }
    inline uint64_t operator * (uint64_t n) const
    {
        auto q = ((m * (uint64_t)(n >> s)) + m) >> N;
        return q >> p;
    }
    inline uint64_t div_slow (uint64_t n) const
    {
        return n / d;
    }
};

inline uint64_t operator / ( uint64_t n, const divisor &d)
{
    return d.div_slow(n);
}
inline uint64_t operator * (uint64_t n, const divisor &d)
{
    return d * n;
}
