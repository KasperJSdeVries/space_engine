#ifndef CORE_DARRAY_H
#define CORE_DARRAY_H

#include "assert.h"

#include <stdlib.h>

#define DARRAY_GROWTH_FACTOR 2
#define DARRAY_START_CAPACITY 8

#define darray_append(arr, val)                                                                    \
    do {                                                                                           \
        if ((arr)->count >= (arr)->capacity) {                                                     \
            (arr)->capacity = (arr)->capacity == 0 ? DARRAY_START_CAPACITY                         \
                                                   : (arr)->capacity * DARRAY_GROWTH_FACTOR;       \
            (arr)->items = realloc((arr)->items, sizeof(*(arr)->items) * (arr)->capacity);         \
            (void)ASSERT((arr)->items != NULL && "Not enough RAM!!!");                             \
        }                                                                                          \
        (arr)->items[(arr)->count++] = (val);                                                      \
    } while (0)

#endif // CORE_DARRAY_H
