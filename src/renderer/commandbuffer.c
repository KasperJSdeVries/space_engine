#include "commandbuffer.h"

#include "device.h"
#include "types.h"

#include "core/assert.h"

#include "vulkan/vulkan_core.h"

b8 commandpool_create(const struct device *device,
                      const struct surface *surface,
                      struct commandpool *commandpool) {
    struct queue_family_indices queue_family_indices =
        find_queue_families(device->physical_device, surface->handle);

    VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue_family_indices.graphics_family,
    };

    if (vkCreateCommandPool(device->handle,
                            &pool_info,
                            NULL,
                            &commandpool->handle) != VK_SUCCESS) {
        return false;
    }

    return true;
}

void commandpool_destroy(const struct device *device,
                         struct commandpool *commandpool) {
    vkDestroyCommandPool(device->handle, commandpool->handle, NULL);
    commandpool->handle = VK_NULL_HANDLE;
}

b8 commandbuffer_create(const struct device *device,
                        const struct commandpool *commandpool,
                        struct commandbuffer *commandbuffer) {
    VkCommandBufferAllocateInfo allocate_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandpool->handle,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    if (vkAllocateCommandBuffers(device->handle,
                                 &allocate_info,
                                 &commandbuffer->handle) != VK_SUCCESS) {
        return false;
    }

    return true;
}

b8 commandbuffer_recording_start(const struct commandbuffer *commandbuffer) {
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };

    if (vkBeginCommandBuffer(commandbuffer->handle, &begin_info) !=
        VK_SUCCESS) {
        return false;
    }

    return true;
}

b8 commandbuffer_recording_end(const struct commandbuffer *commandbuffer) {
    if (vkEndCommandBuffer(commandbuffer->handle) != VK_SUCCESS) {
        return false;
    }

    return true;
}
