#ifndef ASSERT_H
#define ASSERT_H

#include "defines.h"
#include "logging.h"

static inline void _assert(const char *assertion, const char *file, u32 line) {
#if defined(SE_DEBUG)
    LOG_FATAL("Assertion failed at %s:%d: %s", file, line, assertion);
#else
    (void)assertion;
    (void)file;
    (void)line;
#endif
}

#define ASSERT(assertion) ((assertion) ? (true) : (_assert(#assertion, __FILE__, __LINE__), false))

#define ASSERT_UNREACHABLE() _assert("UNREACHABLE", __FILE__, __LINE__)

#endif // ASSERT_H
