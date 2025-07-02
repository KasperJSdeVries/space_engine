#ifndef COMMAND_POOL_H
#define COMMAND_POOL_H

#include "renderer/device.h"

typedef struct {
    VkCommandPool handle;
    const Device *device;
} CommandPool;

CommandPool command_pool_new(const Device *device,
                             u32 queue_family_index,
                             b8 allow_reset);
void command_pool_destroy(CommandPool *command_pool);

#endif // COMMAND_POOL_H
