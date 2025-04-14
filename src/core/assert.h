#ifndef SE_ASSERT_H
#define SE_ASSERT_H

#include "defines.h"
#include "logging.h"

__attribute__((__noreturn__)) static inline void _assert(const char *assertion, const char *file, u32 line) {
    LOG_FATAL("Assertion failed at %s:%d: %s", file, line, assertion);
}

#define ASSERT(assertion) ((assertion) ? (true) : (_assert(#assertion, __FILE__, __LINE__), false))

#define ASSERT_UNREACHABLE() _assert("UNREACHABLE", __FILE__, __LINE__)

#endif // SE_ASSERT_H
