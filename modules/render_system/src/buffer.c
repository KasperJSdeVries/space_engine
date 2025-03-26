#include "buffer.h"
#include "core/assert.h"
#include "vulkan/vulkan_core.h"

u32 find_memory_type(VkPhysicalDevice device, u32 type_filter, VkMemoryPropertyFlags properties);

b8 render_buffer_create(const struct device *device,
                        renderbuffer_usage usage,
                        u64 size,
                        struct renderbuffer *buffer) {
    buffer->size = size;

    VkBufferCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    switch (usage) {
    case RENDERBUFFER_USAGE_VERTEX:
        create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        break;
    case RENDERBUFFER_USAGE_INDEX:
        create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        break;
    case RENDERBUFFER_USAGE_UNIFORM:
        create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        break;
    };

    if (!ASSERT(vkCreateBuffer(device->handle, &create_info, NULL, &buffer->handle) ==
                VK_SUCCESS)) {
        return false;
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(device->handle, buffer->handle, &memory_requirements);

    VkMemoryAllocateInfo allocate_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = find_memory_type(device->physical_device,
                                            memory_requirements.memoryTypeBits,
                                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
    };

    if (!ASSERT(vkAllocateMemory(device->handle, &allocate_info, NULL, &buffer->memory) ==
                VK_SUCCESS)) {
        return false;
    }

    vkBindBufferMemory(device->handle, buffer->handle, buffer->memory, 0);

    return true;
}

void render_buffer_destroy(const struct device *device, struct renderbuffer *buffer) {
    vkDestroyBuffer(device->handle, buffer->handle, NULL);
    vkFreeMemory(device->handle, buffer->memory, NULL);
}

void *render_buffer_map_memory(const struct device *device, const struct renderbuffer *buffer) {
    void *ptr;
    vkMapMemory(device->handle, buffer->memory, 0, buffer->size, 0, &ptr);
    return ptr;
}

void render_buffer_unmap_memory(const struct device *device, const struct renderbuffer *buffer) {
    vkUnmapMemory(device->handle, buffer->memory);
}

u32 find_memory_type(VkPhysicalDevice device, u32 type_filter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(device, &memory_properties);

    for (u32 i = 0; i < memory_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) &&
            (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    ASSERT_UNREACHABLE();
}
