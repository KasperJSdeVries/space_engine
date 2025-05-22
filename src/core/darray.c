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
    u64 header_size = sizeof(darray_header);
    darray_header *header = (darray_header *)((u8 *)array - header_size);

    if (header->capacity == 0) {
        LOG_FATAL("_darray_resize called on a darray with 0 capacity. This is "
                  "not possible.");
        return 0;
    }

    u64 new_array_size =
        header->capacity * DARRAY_GROWTH_FACTOR * header->stride;
    void *new_array = realloc(header, header_size + new_array_size);

    return (void *)((u8 *)new_array + header_size);
}

void *_darray_push(void *array, const void *value_ptr) {
    ASSERT_DEBUG(array);
    u64 header_size = sizeof(darray_header);
    darray_header *header = (darray_header *)((u8 *)array - header_size);

    if (header->length >= header->capacity) {
        array = _darray_resize(array);
        header = (darray_header *)((u8 *)array - header_size);
    }

    u64 address = (u64)array + header->length * header->stride;
    memcpy((void *)address, value_ptr, header->stride);
    header->length++;

    return array;
}

u64 darray_length(void *array) {
    u64 header_size = sizeof(darray_header);
    darray_header *header = (darray_header *)((u8 *)array - header_size);
    return header->length;
}
