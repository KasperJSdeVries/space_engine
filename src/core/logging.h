#ifndef SE_LOGGING_H
#define SE_LOGGING_H

typedef enum {
    LOG_LEVEL_FATAL,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE,
} log_level;

void __attribute__((format(printf, 2, 3))) _log_output(log_level level,
                                                       char *message,
                                                       ...);

#define LOG_FATAL(...) _log_output(LOG_LEVEL_FATAL, __VA_ARGS__)
#define LOG_ERROR(...) _log_output(LOG_LEVEL_ERROR, __VA_ARGS__)
#define LOG_WARN(...) _log_output(LOG_LEVEL_WARN, __VA_ARGS__)
#define LOG_INFO(...) _log_output(LOG_LEVEL_INFO, __VA_ARGS__)
#define LOG_DEBUG(...) _log_output(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define LOG_TRACE(...) _log_output(LOG_LEVEL_TRACE, __VA_ARGS__)

#define TODO(message)                                                          \
    LOG_FATAL("TODO at (%s:%d): %s\n", __FILE__, __LINE__, message)

#endif // SE_LOGGING_H
