#ifndef SINGLE_TIME_COMMANDS_H
#define SINGLE_TIME_COMMANDS_H

#include "renderer/command_buffers.h"
#include "renderer/command_pool.h"
#include "renderer/device.h"

static inline VkCommandBuffer single_time_commands_begin(
    VkCommandBuffer command_buffer) {
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(command_buffer, &begin_info);

    return command_buffer;
}

static inline void single_time_commands_end(CommandPool *command_pool,
                                            VkCommandBuffer command_buffer) {
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer,
    };

    VkQueue graphics_queue = command_pool->device->graphics_queue;

    vkQueueSubmit(graphics_queue, 1, &submit_info, NULL);
    vkQueueWaitIdle(graphics_queue);
}

#define single_time_commands_submit(command_pool)                              \
    CommandBuffers __command_buffers__ = command_buffers_new(command_pool, 1); \
    for (VkCommandBuffer command_buffer = single_time_commands_begin(          \
             command_buffers_get(&__command_buffers__, 0));                    \
         false;                                                                \
         single_time_commands_end(command_pool, command_buffer),               \
                         command_buffers_destroy(&__command_buffers__))

#endif // SINGLE_TIME_COMMANDS_H
