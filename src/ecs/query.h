#ifndef QUERY_H
#define QUERY_H

#include "core/defines.h"

typedef struct {
    u32 count;
    const char **names;
    u64 *sizes;
} query;

query _query_new(const char *first, ...);

// Helper macro to stringify each argument
#define STRINGIFY(x) (#x), (u64)(sizeof(x))
#define EXPAND_AND_STRINGIFY(x) STRINGIFY(x)

#define QUERY_STRUCT_FIELD(type) type *type;
#define QUERY_STRUCT(...)                                                      \
    struct {                                                                   \
        MAP(QUERY_STRUCT_FIELD, __VA_ARGS__)                                   \
    }

// Apply a macro to each argument
#define MAP_1(m, x) m(x)
#define MAP_2(m, x, ...) m(x), MAP_1(m, __VA_ARGS__)
#define MAP_3(m, x, ...) m(x), MAP_2(m, __VA_ARGS__)
#define MAP_4(m, x, ...) m(x), MAP_3(m, __VA_ARGS__)
#define MAP_5(m, x, ...) m(x), MAP_4(m, __VA_ARGS__)
#define MAP_6(m, x, ...) m(x), MAP_5(m, __VA_ARGS__)
#define MAP_7(m, x, ...) m(x), MAP_6(m, __VA_ARGS__)
#define MAP_8(m, x, ...) m(x), MAP_7(m, __VA_ARGS__)
#define MAP_9(m, x, ...) m(x), MAP_8(m, __VA_ARGS__)
#define MAP_10(m, x, ...) m(x), MAP_9(m, __VA_ARGS__)

// Macro dispatcher based on argument count
#define GET_MAP_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, NAME, ...) NAME
#define MAP(m, ...)                                                            \
    GET_MAP_MACRO(__VA_ARGS__,                                                 \
                  MAP_10,                                                      \
                  MAP_9,                                                       \
                  MAP_8,                                                       \
                  MAP_7,                                                       \
                  MAP_6,                                                       \
                  MAP_5,                                                       \
                  MAP_4,                                                       \
                  MAP_3,                                                       \
                  MAP_2,                                                       \
                  MAP_1)(m, __VA_ARGS__)

// Final macro to use
#define query_new(...) _query_new(MAP(EXPAND_AND_STRINGIFY, __VA_ARGS__), NULL)

#endif // QUERY_H
