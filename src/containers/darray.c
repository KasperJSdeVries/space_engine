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
    // LOG_DEBUG("new darray at: %p, with { length: %lu, stride: %lu }",
    //           new_array,
    //           length,
    //           stride);
    memset(new_array, 0, header_size + array_size);

    if (length == 0) {
        LOG_FATAL("_darray_new called with length of 0");
        exit(EXIT_FAILURE);
    }

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

    void *new_array = _darray_new((DARRAY_GROWTH_FACTOR * darray_capacity(array)), darray_stride(array));

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

    if (out_value_ptr != NULL) {
        u64 address = (u64)array + header->length * header->stride;
        memcpy(out_value_ptr, (void *)address, header->stride);
    }
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

    while (index >= darray_capacity(array) || darray_length(array) >= darray_capacity(array)) {
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

void *_darray_duplicate(u64 stride, void *array) {
    u64 header_size = sizeof(darray_header);
    darray_header *source_header = (darray_header *)((u8 *)array - header_size);

    ASSERT_MSG(stride == source_header->stride, "_darray_duplicate: target and source stride mismatch.");

    void *copy = _darray_new(source_header->capacity, stride);
    darray_header *target_header = (darray_header *)((u8 *)copy - header_size);
    ASSERT_MSG(target_header->capacity == source_header->capacity, "capacity mismatch while duplicating darray.");

    target_header->stride = source_header->stride;
    target_header->length = source_header->length;

    memcpy(copy, array, target_header->capacity * target_header->stride);

    return copy;
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

void *_darray_append(void *dst_array, const void *src_array) {
    ASSERT_MSG(darray_stride(src_array) == darray_stride(dst_array), "trying to append array with different stride");

    while (darray_capacity(dst_array) < darray_length(dst_array) + darray_length(src_array)) {
        dst_array = _darray_resize(dst_array);
    }

    u64 src_addr = (u64)src_array;
    u64 dst_addr = (u64)dst_array + darray_length(dst_array) * darray_stride(dst_array);
    memcpy((void *)dst_addr, (void *)src_addr, darray_size(src_array));
    darray_length_set(dst_array, darray_length(dst_array) + darray_length(src_array));

    return dst_array;
}

void darray_clear(void *array) { darray_length_set(array, 0); }

u64 darray_length(const void *array) {
    u64 header_size = sizeof(darray_header);
    darray_header *header = (darray_header *)((u8 *)array - header_size);
    return header->length;
}

void darray_length_set(void *array, u64 length) {
    u64 header_size = sizeof(darray_header);
    darray_header *header = (darray_header *)((u8 *)array - header_size);
    header->length = length;
}

u64 darray_stride(const void *array) {
    u64 header_size = sizeof(darray_header);
    darray_header *header = (darray_header *)((u8 *)array - header_size);
    return header->stride;
}

u64 darray_size(const void *array) { return darray_length(array) * darray_stride(array); }

u64 darray_capacity(const void *array) {
    u64 header_size = sizeof(darray_header);
    darray_header *header = (darray_header *)((u8 *)array - header_size);
    return header->capacity;
}
