
_Pragma("once")

#include <cmath>
#include <limits>
#include <cstdint>
#include <numeric>
#include <type_traits>
template<class T> struct _clz {};
template<> struct _clz<uint32_t> {
    constexpr int operator()(uint32_t t) { return __builtin_clz(t);}
    using restype = int;
};
template<> struct _clz<int32_t> {
    constexpr int operator()(int32_t t) { return __builtin_clz(t);}
    using restype = int;
};
template<> struct _clz<int64_t> {
    constexpr int operator()(int64_t t) { return __builtin_clzl(t);}
    using restype = int;
};
template<> struct _clz<uint64_t> {
    constexpr int operator()(uint64_t t) { return __builtin_clzl(t);}
    using restype = int;
};
template<class T>
constexpr typename _clz<T>::restype clz(T t)
{
    return _clz<T>{}(t);
}
template<class T>
constexpr typename _clz<T>::restype ilog2_by_clz(T x)
{
    constexpr auto digits = std::numeric_limits<T>::digits;
    return digits + 1 - clz(x);
}
template<class T>
constexpr size_t ilog2_naive(T t)
{
    using U = std::make_unsigned_t<T>;
    auto u = U(t) - U(1);
    auto n = 0ul;
    for(; n < std::numeric_limits<U>::digits && (u>>(n)); ++n){}
    return n;
}
template<class T>
constexpr int ilog2_by_cmath_and_cast(T t)
{
    return static_cast<int>(std::floor(std::log2(t)));
}
