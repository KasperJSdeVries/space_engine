#include "device.h"

#include "core/assert.h"
#include "vulkan/vulkan_core.h"

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

b8 queue_familiy_inices_are_complete(struct queue_family_indices indices) {
    return (indices.supported_families & 0xf) == 0xf;
}

i32 rank_physical_device(VkPhysicalDevice);

b8 device_create(const struct instance *instance, struct device *device) {
    u32 physical_device_count;
    (void)vkEnumeratePhysicalDevices(instance->handle, &physical_device_count, NULL);

    if (!ASSERT(physical_device_count > 0)) {
        return false;
    }

    VkPhysicalDevice physical_devices[physical_device_count];
    (void)vkEnumeratePhysicalDevices(instance->handle, &physical_device_count, physical_devices);

    LOG_INFO("Choosing physical device from:");
    i32 highest_score = -1;
    for (u32 i = 0; i < physical_device_count; i++) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physical_devices[i], &properties);

        i32 score = rank_physical_device(physical_devices[i]);

        LOG_INFO("%d: %s, score = %d", i, properties.deviceName, score);
        if (score > highest_score) {
            highest_score = score;
            device->physical_device = physical_devices[i];
        }
    }

    if (!ASSERT(highest_score != -1)) {
        return false;
    }

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device->physical_device, &properties);
    LOG_INFO("Chose device: %s", properties.deviceName);

    return true;
}

void device_destroy(const struct instance *instance, struct device *device) {
    UNUSED(instance);

    vkDestroyDevice(device->handle, NULL);
}

i32 rank_physical_device(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);

    if (features.geometryShader == VK_FALSE) {
        return -1;
    }

    int value = 0;

    switch (properties.deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        value += 10000;
        break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        value += 5000;
        break;
    default:
        break;
    };

    value += properties.limits.maxImageDimension2D;

    return value;
}
