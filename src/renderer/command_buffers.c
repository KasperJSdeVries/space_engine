#include "command_buffers.h"

#include "containers/darray.h"
#include "core/assert.h"
#include "renderer/vulkan.h"
#include "vulkan/vulkan_core.h"

CommandBuffers command_buffers_new(const CommandPool *command_pool, u32 size) {
    CommandBuffers this = {
        .command_pool = command_pool,
        .command_buffers = darray_new(VkCommandBuffer),
    };

    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool->handle,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = size,
    };

    for (u32 i = 0; i < size; i++) {
        darray_push(this.command_buffers, VK_NULL_HANDLE);
    }

    vulkan_check(vkAllocateCommandBuffers(command_pool->device->handle,
                                          &alloc_info,
                                          this.command_buffers),
                 "allocate command buffers");

    return this;
}

void command_buffers_destroy(CommandBuffers *this) {
    if (darray_length(this->command_buffers) > 0) {
        vkFreeCommandBuffers(this->command_pool->device->handle,
                             this->command_pool->handle,
                             darray_length(this->command_buffers),
                             this->command_buffers);
        darray_destroy(this->command_buffers);
    }
}

u32 command_buffers_size(const CommandBuffers *this) {
    return darray_length(this->command_buffers);
}

VkCommandBuffer command_buffers_get(const CommandBuffers *this, u32 index) {
    return this->command_buffers[index];
}

VkCommandBuffer command_buffer_begin(const CommandBuffers *this, u32 index) {
    ASSERT(index < darray_length(this->command_buffers));

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
    };

    vulkan_check(
        vkBeginCommandBuffer(this->command_buffers[index], &begin_info),
        "begin recording command buffer");

    return this->command_buffers[index];
}

void command_buffer_end(const CommandBuffers *this, u32 index) {
    vulkan_check(vkEndCommandBuffer(this->command_buffers[index]),
                 "record command buffer");
}
