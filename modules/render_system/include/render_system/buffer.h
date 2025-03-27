#ifndef RENDER_BUFFER_H
#define RENDER_BUFFER_H

#include "types.h"

typedef enum {
    RENDERBUFFER_USAGE_VERTEX,
    RENDERBUFFER_USAGE_INDEX,
    RENDERBUFFER_USAGE_UNIFORM,
} renderbuffer_usage;

b8 render_buffer_create(const struct device *device,
                        VkBufferUsageFlags usage,
                        VkMemoryPropertyFlags properties,
                        u64 size,
                        struct renderbuffer *buffer);
void render_buffer_destroy(const struct device *device, struct renderbuffer *buffer);
void render_buffer_copy(const struct renderer *renderer,
                        const struct renderbuffer *src_buffer,
                        const struct renderbuffer *dst_buffer);

void render_buffer_map_memory(const struct device *device,
                              const struct renderbuffer *buffer,
                              void **ptr);
void render_buffer_unmap_memory(const struct device *device, const struct renderbuffer *buffer);

#endif // RENDER_BUFFER_H
