#include "darray.h"
#include "core/assert.h"
#include "core/defines.h"
#include "core/logging.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    u64 stride;
    u64 length;
    u64 capacity;
} darray_header;

void *_darray_new(u64 length, u64 stride) {
    u64 header_size = sizeof(darray_header);
    u64 array_size = length * stride;

    void *new_array = malloc(header_size + array_size);
    memset(new_array, 0, header_size + array_size);

    darray_header *header = new_array;
    header->capacity = length;
    header->length = 0;
    header->stride = stride;

    return (void *)((u8 *)new_array + header_size);
}

void darray_destroy(void *array) {
    if (array) {
        u64 header_size = sizeof(darray_header);
        darray_header *header = (darray_header *)((u8 *)array - header_size);
        free(header);
    }
}

void *_darray_resize(void *array) {
    if (darray_capacity(array) == 0) {
        LOG_FATAL("_darray_resize called on a darray with 0 capacity. This is "
                  "not possible.");
        return 0;
    }

    void *new_array =
        _darray_new((DARRAY_GROWTH_FACTOR * darray_capacity(array)),
                    darray_stride(array));

    darray_length_set(new_array, darray_length(array));

    memcpy(new_array, array, darray_size(array));
    darray_destroy(array);

    return new_array;
}

void *_darray_push(void *array, const void *value_ptr) {
    ASSERT_DEBUG(array);

    u64 header_size = sizeof(darray_header);
    darray_header *header = (darray_header *)((u8 *)array - header_size);

    if (header->length >= header->capacity) {
        array = _darray_resize(array);
        header = (darray_header *)((u8 *)array - header_size);
    }

    u64 address = (u64)array + (header->length * header->stride);
    memcpy((void *)address, value_ptr, header->stride);
    darray_length_set(array, header->length + 1);

    return array;
}

void darray_pop(void *array, void *out_value_ptr) {
    ASSERT_DEBUG(array);
    u64 header_size = sizeof(darray_header);
    darray_header *header = (darray_header *)((u8 *)array - header_size);

    header->length--;
    u64 address = (u64)array + header->length * header->stride;
    memcpy(out_value_ptr, (void *)address, header->stride);
}

void darray_pop_front(void *array, void *out_value_ptr) {
    ASSERT_DEBUG(array);
    u64 header_size = sizeof(darray_header);
    darray_header *header = (darray_header *)((u8 *)array - header_size);

    memcpy(out_value_ptr, array, header->stride);
    darray_remove_at_sorted(array, 0);
}

void *_darray_insert_at(void *array, u64 index, const void *value_ptr) {
    ASSERT_DEBUG(array);

    while (index >= darray_capacity(array) ||
           darray_length(array) >= darray_capacity(array)) {
        array = _darray_resize(array);
    }

    u64 address = (u64)array;
    u64 stride = darray_stride(array);
    u64 length = darray_length(array);

    if (index < length) {
        memcpy((void *)(address + ((index + 1) * stride)),
               (void *)(address + (index * stride)),
               stride * (length - index));
        memcpy((void *)(address + (index * stride)), value_ptr, stride);
        darray_length_set(array, length + 1);
    } else {
        memcpy((void *)(address + (index * stride)), value_ptr, stride);
        darray_length_set(array, index + 1);
    }

    return array;
}

void darray_remove_at(void *array, u64 index) {
    ASSERT_DEBUG(array != NULL);

    u64 header_size = sizeof(darray_header);
    darray_header *header = (darray_header *)((u8 *)array - header_size);

    ASSERT(index < header->length);

    memcpy((void *)((u64)array + (index * header->stride)),
           (void *)((u64)array + ((header->length - 1) * header->stride)),
           header->stride);
    darray_length_set(array, header->length - 1);
}

void darray_remove_at_sorted(void *array, u64 index) {
    ASSERT_DEBUG(array != NULL);

    u64 header_size = sizeof(darray_header);
    darray_header *header = (darray_header *)((u8 *)array - header_size);

    ASSERT(index < header->length);

    memcpy((void *)((u64)array + (index * header->stride)),
           (void *)((u64)array + ((index + 1) * header->stride)),
           (header->length - index - 1) * header->stride);
    darray_length_set(array, header->length - 1);
}

void darray_clear(void *array) { darray_length_set(array, 0); }

u64 darray_length(void *array) {
    u64 header_size = sizeof(darray_header);
    darray_header *header = (darray_header *)((u8 *)array - header_size);
    return header->length;
}

void darray_length_set(void *array, u64 length) {
    u64 header_size = sizeof(darray_header);
    darray_header *header = (darray_header *)((u8 *)array - header_size);
    header->length = length;
}

u64 darray_stride(void *array) {
    u64 header_size = sizeof(darray_header);
    darray_header *header = (darray_header *)((u8 *)array - header_size);
    return header->stride;
}

u64 darray_size(void *array) {
    return darray_length(array) * darray_stride(array);
}

u64 darray_capacity(void *array) {
    u64 header_size = sizeof(darray_header);
    darray_header *header = (darray_header *)((u8 *)array - header_size);
    return header->capacity;
}
