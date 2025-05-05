#ifndef _MACROS_H_
#define _MACROS_H_

#if defined(__GNUC__)

#if __SIZEOF_POINTER__ == 8

typedef unsigned long long u64;
typedef long long i64;

typedef unsigned int u32;
typedef int i32;

typedef unsigned short u16;
typedef short i16;

typedef unsigned char u8;
typedef char i8;

typedef float f32;
typedef double f64;

typedef void* any;
typedef u8* raw;

typedef u32* u32ptr;
typedef i32* i32ptr;
typedef u64* u64ptr;
typedef i64* i64ptr;

#endif /* 8 */

#endif /* __GNUC__ */

#define v(pointer) ((void*)(pointer))

#define prefetchl1(addr) asm volatile("prefetcht0 %0" :: "m"(*(const char *)(addr)))
#define prefetchl2(addr) asm volatile("prefetcht1 %0" :: "m"(*(const char *)(addr)))
#define prefetchl3(addr) asm volatile("prefetcht2 %0" :: "m"(*(const char *)(addr)))

#define ali16 __attribute__((aligned(16)))
#define ali32 __attribute__((aligned(32)))
#define ali64 __attribute__((aligned(64)))

#define expect(x)   __builtin_expect(!!(x), 1)
#define uexpect(x) __builtin_expect(!!(x), 0)

#define benchmark                                   \
do {                                                \
    struct timespec __start_time, __end_time;       \
    clock_gettime(CLOCK_MONOTONIC, &__start_time);  \

#define endbench                                    \
    clock_gettime(CLOCK_MONOTONIC, &__end_time);    \
    f64 __elapsed_time =                         \
        (__end_time.tv_sec - __start_time.tv_sec) + \
        (__end_time.tv_nsec - __start_time.tv_nsec) / 1e9; \
    printf("elapsed time: %.12lf seconds\n", __elapsed_time); \
} while (0);

#define rdtsc                                       \
do {                                                \
    u64 __start_cycles;              \
    asm volatile ("rdtsc" : "=A" (__start_cycles));

#define endtsc                                      \
    u64 __end_cycles;                \
    asm volatile ("rdtsc" : "=A" (__end_cycles));   \
    printf("cycles elapsed: %llu\n", __end_cycles - __start_cycles); \
} while (0);

#ifdef __linux__

#define malinfo do {                               \
    u64 __st_mmory = mallinfo2().uordblks;       \

#define endmal                                     \
    printf("memory allocated: %llu bytes\n", mallinfo2().uordblks - __st_mmory); \
} while (0);

#endif /* __linux__ */

#endif /* _MACROS_H_ */
