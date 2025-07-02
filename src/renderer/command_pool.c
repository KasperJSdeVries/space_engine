#include "command_pool.h"

CommandPool command_pool_new(const Device *device,
                             u32 queue_family_index,
                             b8 allow_reset) {
    CommandPool self = {
        .device = device,
    };

    VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = queue_family_index,
        .flags =
            allow_reset ? VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : 0,
    };

    vulkan_check(
        vkCreateCommandPool(device->handle, &pool_info, NULL, &self.handle),
        "create command pool");

    return self;
}

void command_pool_destroy(CommandPool *this) {
    if (this->handle != NULL) {
        vkDestroyCommandPool(this->device->handle, this->handle, NULL);
        this->handle = NULL;
    }
}
