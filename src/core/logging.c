#include "logging.h"

#include <stdarg.h>
#include <stdio.h>

void LOG(LogLevel level, char *message, ...) {
    va_list ap;
    va_start(ap, message);

    char *formatted_message;
    if (vasprintf(&formatted_message, message, ap) == -1) {
        fprintf(stderr,
                "\x1B[31m[ERROR]: Unable to format error string: \"%s\".\x1B[0m\n",
                message);
        return;
    }

    char *color_string = "\x1B[31m";
    char *level_string = "ERROR";
    char *reset_string = "\x1B[0m";

    switch (level) {
    case LOG_LEVEL_INFO:
        color_string = "\x1B[36m";
        level_string = "INFO";
        break;
    case LOG_LEVEL_WARN:
        color_string = "\x1B[33m";
        level_string = "WARN";
        break;
    case LOG_LEVEL_ERROR:
        color_string = "\x1B[31m";
        level_string = "ERROR";
        break;
    }

    fprintf(stderr, "%s[%s]: %s%s\n", color_string, level_string, formatted_message, reset_string);

    free(formatted_message);

    va_end(ap);
}
