#ifndef RENDER_COMMANDBUFFER_H
#define RENDER_COMMANDBUFFER_H

#include "command_pool.h"

typedef struct {
    const CommandPool *command_pool;
    darray(VkCommandBuffer) command_buffers;
} CommandBuffers;

CommandBuffers command_buffers_new(const CommandPool *command_pool, u32 size);
void command_buffers_destroy(CommandBuffers *command_buffers);

u32 command_buffers_size(const CommandBuffers *command_buffers);
VkCommandBuffer command_buffers_get(const CommandBuffers *command_buffers,
                                    u32 index);

VkCommandBuffer command_buffer_begin(const CommandBuffers *command_buffer,
                                     u32 index);
void command_buffer_end(const CommandBuffers *command_buffers, u32 index);

#endif // RENDER_COMMANDBUFFER_H
