#include "logging.h"
#include "core/assert.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void _log_output(LogLevel level,
                 const char *file,
                 u32 line,
                 char *message,
                 ...) {
    const char *level_strings[] =
        {"[FATAL]", "[ERROR]", "[WARN]", "[INFO]", "[DEBUG]", "[TRACE]"};
    const char *color_strings[] =
        {"0;41", "1;31", "1;33", "1;32", "1;34", "1;37"};

    va_list ap;
    va_start(ap, message);

#if defined(SE_LINUX)
    char *out_message;
    if (vasprintf(&out_message, message, ap) == -1) {
        fprintf(stderr,
                "\033[0;41m[FATAL] failed to format _log_output\033[0m\n");
        return;
    };
#else
    u64 out_message_size = vsnprintf(NULL, 0, message, ap);
    char *out_message = malloc(out_message_size + 1);
    if (vsnprintf(out_message, out_message_size + 1, message, ap) == -1) {
        fprintf(stderr,
                "\033[0;41m[FATAL] failed to format _log_output\033[0m\n");
        return;
    };
#endif

    b8 is_error = (level == LOG_LEVEL_ERROR || level == LOG_LEVEL_FATAL);
    FILE *console_handle = is_error ? stderr : stdout;

    const char *filename = strrchr(file, '/') + 1;

    fprintf(console_handle,
            "\033[%sm%s %s \033[1;97m(%s:%d)\033[0m\n",
            color_strings[level],
            level_strings[level],
            out_message,
            filename,
            line);

    free(out_message);

    va_end(ap);
}

void report_assertion_failure(const char *expression,
                              const char *message,
                              const char *file,
                              i32 line) {
    _log_output(LOG_LEVEL_FATAL,
                file,
                line,
                "Assertion Failure: %s, message: %s",
                expression,
                message);
}
