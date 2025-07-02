#ifndef BUFFER_H
#define BUFFER_H

#include "containers/darray.h"
#include "renderer/command_pool.h"
#include "renderer/device.h"
#include "renderer/device_memory.h"
#include "vulkan/vulkan_core.h"
#include <string.h>

typedef struct {
    VkBuffer handle;
    const Device *device;
} Buffer;

Buffer buffer_new(const Device *device, u64 size, VkBufferUsageFlags usage);
void buffer_destroy(Buffer *self);

DeviceMemory buffer_allocate_memory(const Buffer *self,
                                    VkMemoryAllocateFlags allocate_flags,
                                    VkMemoryPropertyFlags property_flags);
VkMemoryRequirements buffer_get_memory_requirements(const Buffer *buffer);
VkDeviceAddress buffer_get_device_address(const Buffer *buffer);

void buffer_copy_from(const Buffer *self,
                      CommandPool *command_pool,
                      const Buffer *src,
                      VkDeviceSize size);

static inline void create_device_buffer(CommandPool *command_pool,
                                        VkBufferUsageFlags usage,
                                        darray(void) array,
                                        Buffer *buffer,
                                        DeviceMemory *memory) {
    const Device *device = command_pool->device;
    u64 content_size = darray_length(array) * darray_stride(array);
    VkMemoryAllocateFlags allocate_flags =
        usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
            ? VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
            : 0;

    buffer_destroy(buffer);
    device_memory_destroy(memory);
    *buffer = buffer_new(device,
                         content_size,
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage);
    *memory = buffer_allocate_memory(buffer,
                                     allocate_flags,
                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    Buffer staging_buffer =
        buffer_new(device, content_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    DeviceMemory staging_buffer_memory =
        buffer_allocate_memory(&staging_buffer,
                               0,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *data = device_memory_map(&staging_buffer_memory, 0, content_size);
    memcpy(data, array, content_size);
    device_memory_unmap(&staging_buffer_memory);

    buffer_copy_from(buffer, command_pool, &staging_buffer, content_size);

    buffer_destroy(&staging_buffer);
    device_memory_destroy(&staging_buffer_memory);
}

#endif // BUFFER_H
