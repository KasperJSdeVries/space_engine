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
        typeof(*array) __temp_value_copy__ = value;                            \
        array = _darray_push(array, &__temp_value_copy__);                     \
    } while (0)

void darray_pop(void *array, void *out_value_ptr);

void darray_pop_front(void *array, void *out_value_ptr);

void *_darray_insert_at(void *array, u64 index, const void *value_ptr);

#define darray_insert_at(array, index, value)                                  \
    do {                                                                       \
        typeof(*array) __temp_value_copy__ = value;                            \
        array = _darray_insert_at(array, index, &__temp_value_copy__);         \
    } while (0)

void *_darray_duplicate(u64 stride, void *array);

#define darray_duplicate(type, array)                                          \
    (type *)_darray_duplicate(sizeof(type), array)

void darray_remove_at(void *array, u64 index);

void darray_remove_at_sorted(void *array, u64 index);

void *_darray_append(void *dst_array, const void *src_array);
#define darray_append(dst, src) dst = _darray_append(dst, src)

void darray_clear(void *array);

u64 darray_length(const void *array);

void darray_length_set(void *array, u64 length);

u64 darray_stride(const void *array);

u64 darray_size(const void *array);

u64 darray_capacity(const void *array);

#endif // CORE_DARRAY_H
