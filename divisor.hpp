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
    constexpr divisor() = default;
    constexpr divisor(const divisor&) = default;
    constexpr divisor(divisor&&)noexcept = default;
    divisor& operator=(const divisor&) = default;
    divisor& operator=(divisor&&)noexcept = default;
    divisor(uint64_t _d, uint64_t _N = 64)
    : N(_N)
    , d(_d)
    {
        for(; d >> p && p < N - 1ul; p++) {}
        for(; d >> s && !((d>>s)&1) && s < N - 1ul; s++) {}
        m = ((uint128_t)1 << ( N + p)) / ((uint128_t)(d >> s));
    }
    constexpr uint64_t operator * (uint64_t n) const
    {
        return (((m * (uint128_t)(n >> s)) + m) >> N ) >> p;
    }
    constexpr uint64_t div_slow (uint64_t n) const
    {
        return n / d;
    }
};

constexpr uint64_t operator / ( uint64_t n, const divisor &d)
{
    return d.div_slow(n);
}
constexpr uint64_t operator * (uint64_t n, const divisor &d)
{
    return d * n;
}

std::ostream &operator << (std::ostream &ost, divisor &div)
{
    ost << "<divisor representing n / " << div.d << " by computing " << std::endl
        << " ( ( ( ( " << (uint64_t)div.m << " ) * ( n >> " << div.s << " ) ) + " 
        << (uint64_t)div.m << " ) >> " << div.N << " ) >> " << div.p << "  > " << std::endl;
}
