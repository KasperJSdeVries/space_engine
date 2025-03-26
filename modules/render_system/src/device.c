#include "device.h"

#include "instance.h"
#include "swapchain.h"

#include "core/assert.h"
#include "core/defines.h"
#include "core/logging.h"

#include <vulkan/vulkan_core.h>

#include <stdio.h>
#include <string.h>

static const char *device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
static const u32 device_extension_count = ARRAY_SIZE(device_extensions);

static i32 rank_physical_device(VkPhysicalDevice device, VkSurfaceKHR surface);
static b8 check_device_extension_support(VkPhysicalDevice device);

b8 device_create(const struct instance *instance, VkSurfaceKHR surface, struct device *device) {
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

        i32 score = rank_physical_device(physical_devices[i], surface);

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

    struct queue_family_indices indices = find_queue_families(device->physical_device, surface);
    u32 indices_array[] = {indices.graphics_family,
                           indices.compute_family,
                           indices.present_family,
                           indices.transfer_family};
    u32 unique_indices[ARRAY_SIZE(indices_array)];
    u32 unique_indices_count = 0;
    for (u32 i = 0; i < 4; i++) {
        b8 exists = false;
        for (u32 j = 0; j < unique_indices_count; j++) {
            if (indices_array[i] == unique_indices[j]) {
                exists = true;
            }
        }
        if (!exists) {
            unique_indices[unique_indices_count++] = indices_array[i];
        }
    }

    VkDeviceQueueCreateInfo queue_create_infos[unique_indices_count];
    memset(queue_create_infos, 0, sizeof(queue_create_infos));

    f32 queue_priorities[] = {1.0f};
    for (u32 i = 0; i < unique_indices_count; i++) {
        queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[i].queueFamilyIndex = unique_indices[i];
        queue_create_infos[i].queueCount = 1;
        queue_create_infos[i].pQueuePriorities = queue_priorities;
    }

    VkPhysicalDeviceFeatures device_features = {0};

    VkDeviceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = unique_indices_count,
        .pQueueCreateInfos = queue_create_infos,
        .pEnabledFeatures = &device_features,
        .enabledExtensionCount = device_extension_count,
        .ppEnabledExtensionNames = device_extensions,
#if ENABLE_VALIDATION_LAYERS
        .enabledLayerCount = validation_layer_count,
        .ppEnabledLayerNames = validation_layers,
#endif
    };

    VkResult result = vkCreateDevice(device->physical_device, &create_info, NULL, &device->handle);
    if (!ASSERT(result == VK_SUCCESS)) {
        return false;
    }

    vkGetDeviceQueue(device->handle, indices.graphics_family, 0, &device->graphics_queue);
    vkGetDeviceQueue(device->handle, indices.present_family, 0, &device->present_queue);
    vkGetDeviceQueue(device->handle, indices.transfer_family, 0, &device->transfer_queue);
    vkGetDeviceQueue(device->handle, indices.compute_family, 0, &device->compute_queue);

    return true;
}

void device_destroy(struct device *device) { vkDestroyDevice(device->handle, NULL); }

b8 queue_familiy_indices_are_complete(struct queue_family_indices indices) {
    return (indices.supported_families & 0xf) == 0xf;
}

struct queue_family_indices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface) {
    struct queue_family_indices indices;

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    LOG_INFO("Queue family properties for: %s", properties.deviceName);

    u32 queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);
    VkQueueFamilyProperties queue_families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

    LOG_INFO("                 Graphics | Compute | Transfer | Present");
    for (u32 i = 0; i < queue_family_count; i++) {
        if (!(indices.supported_families & QUEUE_FAMILY_SUPPORT_GRAPHICS_BIT) &&
            (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            indices.graphics_family = i;
            indices.supported_families |= QUEUE_FAMILY_SUPPORT_GRAPHICS_BIT;
        }

        if (!(indices.supported_families & QUEUE_FAMILY_SUPPORT_COMPUTE_BIT) &&
            (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            indices.graphics_family = i;
            indices.supported_families |= QUEUE_FAMILY_SUPPORT_COMPUTE_BIT;
        }

        if (!(indices.supported_families & QUEUE_FAMILY_SUPPORT_TRANSFER_BIT) &&
            (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT)) {
            indices.graphics_family = i;
            indices.supported_families |= QUEUE_FAMILY_SUPPORT_TRANSFER_BIT;
        }

        VkBool32 present_support = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
        if (!(indices.supported_families & QUEUE_FAMILY_SUPPORT_PRESENT_BIT) && present_support) {
            indices.present_family = i;
            indices.supported_families |= QUEUE_FAMILY_SUPPORT_PRESENT_BIT;
        }

        LOG_INFO("Queue family #%1d: %s | %s | %s | %s",
                 i,
                 (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) ? "   x    " : "        ",
                 (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) ? "   x   " : "       ",
                 (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) ? "   x    " : "        ",
                 present_support ? "   x   " : "       ");
    }

    return indices;
}

static i32 rank_physical_device(VkPhysicalDevice device, VkSurfaceKHR surface) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);

    struct queue_family_indices indices = find_queue_families(device, surface);
    if (!queue_familiy_indices_are_complete(indices)) {
        return -1;
    }

    if (!check_device_extension_support(device)) {
        return -1;
    }

    struct swapchain_support_details swapchain_support = query_swapchain_support(device, surface);
    if (swapchain_support.format_count == 0 || swapchain_support.present_mode_count == 0) {
        return -1;
    }
    swapchain_support_details_destroy(&swapchain_support);

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

static b8 check_device_extension_support(VkPhysicalDevice device) {
    u32 extension_count;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);
    VkExtensionProperties available_extensions[extension_count];
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, available_extensions);

    for (u32 i = 0; i < device_extension_count; i++) {
        b8 found = false;
        for (u32 j = 0; j < extension_count; j++) {
            if (strncmp(available_extensions[j].extensionName, device_extensions[i], 256) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }

    return true;
}
