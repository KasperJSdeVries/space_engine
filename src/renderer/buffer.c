#include "buffer.h"
#include "core/assert.h"
#include "vulkan/vulkan_core.h"

u32 find_memory_type(VkPhysicalDevice device,
                     u32 type_filter,
                     VkMemoryPropertyFlags properties);

b8 render_buffer_create(const struct device *device,
                        VkBufferUsageFlags usage,
                        VkMemoryPropertyFlags properties,
                        u64 size,
                        struct renderbuffer *buffer) {
    buffer->size = size;

    VkBufferCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .usage = usage,
    };

    if (vkCreateBuffer(device->handle, &create_info, NULL, &buffer->handle) !=
        VK_SUCCESS) {
        return false;
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(device->handle,
                                  buffer->handle,
                                  &memory_requirements);

    VkMemoryAllocateInfo allocate_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = find_memory_type(device->physical_device,
                                            memory_requirements.memoryTypeBits,
                                            properties),
    };

    if (vkAllocateMemory(device->handle,
                         &allocate_info,
                         NULL,
                         &buffer->memory) != VK_SUCCESS) {
        return false;
    }

    vkBindBufferMemory(device->handle, buffer->handle, buffer->memory, 0);

    return true;
}

void render_buffer_destroy(const struct device *device,
                           struct renderbuffer *buffer) {
    vkDestroyBuffer(device->handle, buffer->handle, NULL);
    vkFreeMemory(device->handle, buffer->memory, NULL);
}

void render_buffer_copy(const struct renderer *renderer,
                        const struct renderbuffer *src_buffer,
                        const struct renderbuffer *dst_buffer) {
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = renderer->commandpool.handle,
        .commandBufferCount = 1,
    };

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(renderer->device.handle,
                             &alloc_info,
                             &command_buffer);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(command_buffer, &begin_info);

    VkBufferCopy copy_region = {
        .size = src_buffer->size,
    };

    vkCmdCopyBuffer(command_buffer,
                    src_buffer->handle,
                    dst_buffer->handle,
                    1,
                    &copy_region);

    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer,
    };

    vkQueueSubmit(renderer->device.graphics_queue,
                  1,
                  &submit_info,
                  VK_NULL_HANDLE);
    vkQueueWaitIdle(renderer->device.graphics_queue);
}

void render_buffer_map_memory(const struct device *device,
                              const struct renderbuffer *buffer,
                              void **ptr) {
    vkMapMemory(device->handle, buffer->memory, 0, buffer->size, 0, ptr);
}

void render_buffer_unmap_memory(const struct device *device,
                                const struct renderbuffer *buffer) {
    vkUnmapMemory(device->handle, buffer->memory);
}

u32 find_memory_type(VkPhysicalDevice device,
                     u32 type_filter,
                     VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(device, &memory_properties);

    for (u32 i = 0; i < memory_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) &&
            (memory_properties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
            return i;
        }
    }

    ASSERT_UNREACHABLE();
}
