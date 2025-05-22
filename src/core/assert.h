#ifndef SE_ASSERT_H
#define SE_ASSERT_H

#include "defines.h"

#include <stdlib.h>

#if defined(__has_builtin) && !defined(__ibmxl__)
    #if __has_builtin(__builtin_debugtrap)
        #define debug_break() __builtin_debugtrap()
    #elif __has_builtin(__debugbreak)
        #define debug_break() __debugbreak()
    #endif
#endif

#if !defined(debug_break)
    #if defined(__clang__) || defined(__gcc__)
        #define debug_break() __builtin_trap()
    #elif defined(_MSC_VER)
        #include <intrin.h>
        #define debug_break() __debugbreak()
    #else
        #define debug_break() // asm { int 3 }
    #endif
#endif

void report_assertion_failure(const char *expression,
                              const char *message,
                              const char *file,
                              i32 line);

#define ASSERT(expression)                                                     \
    do {                                                                       \
        if (expression) {                                                      \
        } else {                                                               \
            report_assertion_failure(#expression, "", __FILE__, __LINE__);     \
            debug_break();                                                     \
        }                                                                      \
    } while (0)

#define ASSERT_MSG(expression, message)                                        \
    do {                                                                       \
        if (expression) {                                                      \
        } else {                                                               \
            report_assertion_failure(#expression,                              \
                                     message,                                  \
                                     __FILE__,                                 \
                                     __LINE__);                                \
            debug_break();                                                     \
        }                                                                      \
    } while (0)

#ifdef SE_DEBUG
    #define ASSERT_DEBUG(expression)                                           \
        do {                                                                   \
            if (expression) {                                                  \
            } else {                                                           \
                report_assertion_failure(#expression, "", __FILE__, __LINE__); \
                debug_break();                                                 \
            }                                                                  \
        } while (0)
#else
    #define ASSERT_DEBUG(expression)
#endif

#define ASSERT_UNREACHABLE()                                                   \
    do {                                                                       \
        report_assertion_failure("", "UNREACHABLE", __FILE__, __LINE__);       \
        debug_break();                                                         \
        exit(EXIT_FAILURE);                                                    \
    } while (0)

#endif // SE_ASSERT_H
