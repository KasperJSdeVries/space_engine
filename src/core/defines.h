#ifndef SE_DEFINES_H
#define SE_DEFINES_H

#define UNUSED(value) (void)(value)

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof(*(arr)))

#define UNLIKELY(expr) __builtin_expect(!!(expr), 0)

#if !defined(SE_WIN32) && !defined(SE_LINUX)
    #if defined(WIN32) || defined(_WIN32)
        #define SE_WIN32
    #endif
    #if defined(__linux__)
        #define SE_LINUX
    #endif
#endif

#if defined(__clang__) || defined(__GNUC__)
    #define STATIC_ASSERT _Static_assert
#else
    #define STATIC_ASSERT static_assert
#endif

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

STATIC_ASSERT(sizeof(u8) == 1, "Expected u8 to be 1 byte.");
STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 byte.");
STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 byte.");
STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 byte.");

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long i64;

STATIC_ASSERT(sizeof(i8) == 1, "Expected i8 to be 1 byte.");
STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 byte.");
STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 byte.");
STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 byte.");

typedef float f32;
typedef double f64;

#define F32_EPSILON 1.19209290e-7F
#define F64_EPSILON 2.2204460492503131e-16

STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 byte.");
STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 byte.");

#define atomic _Atomic

typedef _Bool b8;
typedef int b32;

#define true 1
#define false 0

STATIC_ASSERT(sizeof(b8) == 1, "Expected b8 to be 1 byte.");
STATIC_ASSERT(sizeof(b32) == 4, "Expected b32 to be 4 byte.");

#define PI 3.14159265358979323846

#define DEG2RAD(deg) ((typeof(deg))((deg) * PI / 180.0))
#define RAD2DEG(rad) ((typeof(rad))((rad) * 180.0 / PI))

#define CLAMP(val, min, max)                                                   \
    (((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)))

#endif // SE_DEFINES_H
