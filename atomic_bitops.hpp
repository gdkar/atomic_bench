_Pragma("once")

#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <atomic>
#include <x86intrin.h>

template<typename T>
bool atomic_test_and_set_bit(T *addr, int64_t bit)
{
    int old;
    asm volatile("lock; bts %2, %1;\n\t"
                 "sbb %0, %0;\n\t"
               : "=r" (old), "+m" (*addr)
               : "Ir" (bit)
               : "cc", "memory");
    return old;
}
template<typename T>
bool atomic_test_and_clear_bit(T *addr, int64_t bit)
{
    int old;
    asm volatile("lock; btr %2, %1;\n\t"
                 "sbb %0, %0;\n\t"
               : "=r" (old), "+m" (*addr)
               : "Ir" (bit)
               : "cc", "memory");
    return old;
}
template<typename T>
bool test_and_set_bit(T *addr, int64_t bit)
{
    int old;
    asm volatile("bts %2, %1;\n\t"
                 "sbb %0, %0;\n\t"
               : "=r" (old), "+m" (*addr)
               : "Ir" (bit)
               : "cc", "memory");
    return old;
}
template<typename T>
bool test_and_clear_bit(T *addr, int64_t bit)
{
    int old;
    asm volatile("btr %2, %1;\n\t"
                 "sbb %0, %0;\n\t"
               : "=rq" (old), "+m" (*addr)
               : "Irq" (bit)
               : "cc", "memory");
    return old;
}
