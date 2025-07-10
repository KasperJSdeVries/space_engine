#ifndef SE_LOGGING_H
#define SE_LOGGING_H

#include "core/defines.h"

typedef enum {
    LOG_LEVEL_FATAL,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE,
} LogLevel;

void __attribute__((format(printf, 4, 5))) _log_output(LogLevel level,
                                                       const char *file,
                                                       u32 line,
                                                       char *message,
                                                       ...);

#define LOG_FATAL(...)                                                         \
    _log_output(LOG_LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...)                                                         \
    _log_output(LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...)                                                          \
    _log_output(LOG_LEVEL_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...)                                                          \
    _log_output(LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...)                                                         \
    _log_output(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_TRACE(...)                                                         \
    _log_output(LOG_LEVEL_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define LOG(level, ...) _log_output(level, __FILE__, __LINE__, __VA_ARGS__)

#define TODO(message) LOG_FATAL("TODO: %s", message)

#endif // SE_LOGGING_H
