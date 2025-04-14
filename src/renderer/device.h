#ifndef RENDER_DEVICE_H
#define RENDER_DEVICE_H

#include "types.h"

b8 device_create(const struct instance *instance,
                 const struct surface *surface,
                 struct device *device);
void device_destroy(struct device *device);

enum queue_family_support_bits {
    QUEUE_FAMILY_SUPPORT_GRAPHICS_BIT = 1 << 0,
    QUEUE_FAMILY_SUPPORT_PRESENT_BIT = 1 << 1,
    QUEUE_FAMILY_SUPPORT_TRANSFER_BIT = 1 << 2,
    QUEUE_FAMILY_SUPPORT_COMPUTE_BIT = 1 << 3,
    QUEUE_FAMILY_SUPPORT_MAX_ENUM,
};

struct queue_family_indices {
    u32 graphics_family;
    u32 present_family;
    u32 compute_family;
    u32 transfer_family;
    u8 supported_families;
};

b8 queue_familiy_indices_are_complete(struct queue_family_indices indices);
struct queue_family_indices find_queue_families(VkPhysicalDevice device,
                                                VkSurfaceKHR surface);

#endif // RENDER_DEVICE_H
