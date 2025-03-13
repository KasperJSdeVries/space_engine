#ifndef SE_LOGGING_H
#define SE_LOGGING_H

#include <stdlib.h>

typedef enum {
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
} LogLevel;

void __attribute__((format(printf, 2, 3))) LOG(LogLevel level, char *message, ...);
#define LOG_INFO(...) LOG(LOG_LEVEL_INFO, __VA_ARGS__)
#define LOG_WARN(...) LOG(LOG_LEVEL_WARN, __VA_ARGS__)
#define LOG_ERROR(...) LOG(LOG_LEVEL_ERROR, __VA_ARGS__)
#define LOG_FATAL(...)                                                                             \
    do {                                                                                           \
        LOG(LOG_LEVEL_ERROR, __VA_ARGS__);                                                         \
        exit(EXIT_FAILURE);                                                                        \
    } while (0)

#endif // SE_LOGGING_H
