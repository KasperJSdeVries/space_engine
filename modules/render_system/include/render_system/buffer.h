#ifndef RENDER_BUFFER_H
#define RENDER_BUFFER_H

#include "types.h"

struct render_buffer {
    VkBuffer handle;
    VkDeviceMemory memory;
    u64 size;
};

b8 render_buffer_create(const struct device *device, u64 size, struct render_buffer *buffer);
void render_buffer_destroy(const struct device *device, struct render_buffer *buffer);

void *render_buffer_map_memory(const struct render_buffer *buffer);
void render_buffer_unmap_memory(const struct render_buffer *buffer);

#endif // RENDER_BUFFER_H
