#include "query.h"

#include <stdarg.h>
#include <string.h>

query _query_new(const char *first, ...) {
    va_list args;

    u32 count = 0;
    const char *str = first;
    u64 size;

    va_start(args, first);
    while (str != NULL) {
        size = va_arg(args, u64);
        count++;
        str = va_arg(args, const char *);
    }
    va_end(args);

    const char **names = malloc(sizeof(char *) * count);
    u64 *sizes = malloc(sizeof(u64) * count);

    va_start(args, first);
    str = first;
    for (u32 i = 0; i < count; i++) {
        size = va_arg(args, u64);
        names[i] = str;
        sizes[i] = size;
        str = va_arg(args, const char *);
    }
    va_end(args);

    return (query){
        .names = names,
        .count = count,
        .sizes = sizes,
    };
}
