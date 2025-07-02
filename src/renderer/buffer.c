#include "buffer.h"
#include "renderer/device_memory.h"
#include "renderer/single_time_commands.h"
#include "renderer/vulkan.h"
#include "vulkan/vulkan_core.h"

Buffer buffer_new(const Device *device, u64 size, VkBufferUsageFlags usage) {
    Buffer self = {
        .device = device,
    };

    VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    vulkan_check(
        vkCreateBuffer(device->handle, &buffer_info, NULL, &self.handle),
        "create buffer");

    return self;
}

void buffer_destroy(Buffer *self) {
    if (self->handle != NULL) {
        vkDestroyBuffer(self->device->handle, self->handle, NULL);
        self->handle = NULL;
    }
}

DeviceMemory buffer_allocate_memory(const Buffer *self,
                                    VkMemoryAllocateFlags allocate_flags,
                                    VkMemoryPropertyFlags property_flags) {
    VkMemoryRequirements requirements = buffer_get_memory_requirements(self);
    DeviceMemory memory = device_memory_new(self->device,
                                            requirements.size,
                                            requirements.memoryTypeBits,
                                            allocate_flags,
                                            property_flags);

    vulkan_check(vkBindBufferMemory(self->device->handle,
                                    self->handle,
                                    memory.handle,
                                    0),
                 "bind buffer memory");

    return memory;
}

VkMemoryRequirements buffer_get_memory_requirements(const Buffer *self) {
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(self->device->handle,
                                  self->handle,
                                  &requirements);
    return requirements;
}

VkDeviceAddress buffer_get_device_address(const Buffer *self) {
    VkBufferDeviceAddressInfo info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .pNext = NULL,
        .buffer = self->handle,
    };

    return vkGetBufferDeviceAddress(self->device->handle, &info);
}

void buffer_copy_from(const Buffer *self,
                      CommandPool *command_pool,
                      const Buffer *src,
                      VkDeviceSize size) {
    single_time_commands_submit(command_pool) {
        VkBufferCopy copy_region = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size,
        };

        vkCmdCopyBuffer(command_buffer,
                        src->handle,
                        self->handle,
                        1,
                        &copy_region);
    }
}
