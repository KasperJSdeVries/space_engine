#include "device_memory.h"

#include "core/logging.h"
#include "renderer/device.h"
#include "renderer/vulkan.h"

#include <stdlib.h>

static u32 find_memory_type(VkPhysicalDevice physical_device,
                            u32 memory_type_bits,
                            VkMemoryPropertyFlags property_flags);

DeviceMemory device_memory_new(const Device *device,
                               u64 size,
                               u32 memory_type_bits,
                               VkMemoryAllocateFlags allocate_flags,
                               VkMemoryPropertyFlags property_flags) {
    DeviceMemory self = {
        .device = device,
    };

    VkMemoryAllocateFlagsInfo flags_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
        .flags = allocate_flags,
    };

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = &flags_info,
        .allocationSize = size,
        .memoryTypeIndex = find_memory_type(device->physical_device,
                                            memory_type_bits,
                                            property_flags),
    };

    vulkan_check(
        vkAllocateMemory(device->handle, &alloc_info, NULL, &self.handle),
        "allocate memory");

    return self;
}

DeviceMemory device_memory_move(DeviceMemory *other) {
    DeviceMemory self = {
        .device = other->device,
        .handle = other->handle,
    };

    other->handle = NULL;

    return self;
}

void device_memory_destroy(DeviceMemory *self) {
    if (self->handle != NULL) {
        vkFreeMemory(self->device->handle, self->handle, NULL);
        self->handle = NULL;
    }
}

void *device_memory_map(DeviceMemory *self, u64 offset, u64 size) {
    void *data;
    vulkan_check(
        vkMapMemory(self->device->handle, self->handle, offset, size, 0, &data),
        "map memory");
    return data;
}

void device_memory_unmap(DeviceMemory *self) {
    vkUnmapMemory(self->device->handle, self->handle);
}

static u32 find_memory_type(VkPhysicalDevice physical_device,
                            u32 type_filter,
                            VkMemoryPropertyFlags property_flags) {
    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_props);

    for (u32 i = 0; i != mem_props.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) &&
            (mem_props.memoryTypes[i].propertyFlags & property_flags) ==
                property_flags) {
            return i;
        }
    }

    LOG_FATAL("failed to find suitable memory type");
    exit(EXIT_FAILURE);
}
