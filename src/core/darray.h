#ifndef CORE_DARRAY_H
#define CORE_DARRAY_H

#include "core/defines.h"

#define DARRAY_DEFAULT_CAPACITY 1
#define DARRAY_GROWTH_FACTOR 2

#define darray(...) __VA_ARGS__ *

void *_darray_new(u64 capacity, u64 stride);

#define darray_new(type)                                                       \
    (type *)_darray_new(DARRAY_DEFAULT_CAPACITY, sizeof(type))

void darray_destroy(void *array);

void *_darray_resize(void *array);

void *_darray_push(void *array, const void *value_ptr);

#define darray_push(array, value)                                              \
    do {                                                                       \
        typeof(value) __temp_value_copy__ = value;                             \
        array = _darray_push(array, &__temp_value_copy__);                     \
    } while (0)

u64 darray_length(void *array);

#endif // CORE_DARRAY_H
