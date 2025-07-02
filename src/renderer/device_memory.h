#ifndef DEVICE_MEMORY_H
#define DEVICE_MEMORY_H

#include "core/defines.h"
#include "renderer/device.h"

typedef struct {
    const Device *device;
    VkDeviceMemory handle;
} DeviceMemory;

DeviceMemory device_memory_new(const Device *device,
                               u64 size,
                               u32 memory_type_bits,
                               VkMemoryAllocateFlags allocate_flags,
                               VkMemoryPropertyFlags property_flags);
DeviceMemory device_memory_move(DeviceMemory *other);
void device_memory_destroy(DeviceMemory *self);

void *device_memory_map(DeviceMemory *self, u64 offset, u64 size);
void device_memory_unmap(DeviceMemory *self);

#endif // DEVICE_MEMORY_H
