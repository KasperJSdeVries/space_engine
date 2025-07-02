#include "uniform_buffer.h"

#include "renderer/buffer.h"
#include "renderer/device_memory.h"

#include <string.h>

UniformBuffer uniform_buffer_new(const Device *device) {
    u64 buffer_size = sizeof(UniformBufferObject);

    Buffer buffer =
        buffer_new(device, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    DeviceMemory memory =
        buffer_allocate_memory(&buffer,
                               0,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    return (UniformBuffer){buffer, memory};
}

void uniform_buffer_destroy(UniformBuffer *self) {
    buffer_destroy(&self->buffer);
    device_memory_destroy(&self->memory);
}

void uniform_buffer_set_value(UniformBuffer *self, UniformBufferObject value) {
    void *data =
        device_memory_map(&self->memory, 0, sizeof(UniformBufferObject));
    memcpy(data, &value, sizeof(UniformBufferObject));
    device_memory_unmap(&self->memory);
}
