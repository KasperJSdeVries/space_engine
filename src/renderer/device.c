#include "device.h"

#include "containers/darray.h"
#include "core/defines.h"
#include "core/logging.h"
#include "renderer/instance.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static u32 find_queue(VkQueueFamilyProperties *queue_families,
                      u32 queue_family_count,
                      const char *name,
                      VkQueueFlags required_bits,
                      VkQueueFlags excluded_bits);
static void check_required_extensions(VkPhysicalDevice device,
                                      darray(const char *) required_extensions);

Device device_new(VkPhysicalDevice physical_device,
                  const Surface *surface,
                  darray(const char *) required_extensions,
                  VkPhysicalDeviceFeatures device_features,
                  const void *next_device_features) {
    check_required_extensions(physical_device, required_extensions);

    u32 queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
                                             &queue_family_count,
                                             NULL);
    VkQueueFamilyProperties queue_families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
                                             &queue_family_count,
                                             queue_families);
    u32 graphics_family = find_queue(queue_families,
                                     queue_family_count,
                                     "graphics",
                                     VK_QUEUE_GRAPHICS_BIT,
                                     0);
    u32 compute_family = find_queue(queue_families,
                                    queue_family_count,
                                    "compute",
                                    VK_QUEUE_COMPUTE_BIT,
                                    VK_QUEUE_GRAPHICS_BIT);
    u32 transfer_family =
        find_queue(queue_families,
                   queue_family_count,
                   "transfer",
                   VK_QUEUE_TRANSFER_BIT,
                   VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

    b8 present_family_found = false;
    u32 present_family = 0;
    for (u32 i = 0; i < queue_family_count; i++) {
        VkBool32 present_support = VK_FALSE;
        vulkan_check(vkGetPhysicalDeviceSurfaceSupportKHR(physical_device,
                                                          i,
                                                          surface->handle,
                                                          &present_support),
                     "get physical device surface support khr");
        LOG_DEBUG("present_support[%d]: %s",
                  i,
                  present_support ? "true" : "false");
        if (present_support && queue_families[i].queueCount > 0) {
            present_family_found = true;
            present_family = i;
            break;
        }
    }
    if (!present_family_found) {
        LOG_FATAL("found no presentation queue");
        exit(EXIT_FAILURE);
    }

    u32 chosen_queue_families[] = {
        graphics_family,
        compute_family,
        present_family,
        transfer_family,
    };

    darray(u32) unique_queue_families = darray_new(u32);
    for (u32 i = 0; i < 4; i++) {
        b8 exists = false;
        for (u32 j = 0; j < darray_length(unique_queue_families); j++) {
            if (chosen_queue_families[i] == unique_queue_families[j]) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            darray_push(unique_queue_families, chosen_queue_families[i]);
        }
    }

    f32 queue_priority = 1.0f;
    darray(VkDeviceQueueCreateInfo) queue_create_infos =
        darray_new(VkDeviceQueueCreateInfo);

    for (u32 i = 0; i < darray_length(unique_queue_families); i++) {
        VkDeviceQueueCreateInfo queue_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = unique_queue_families[i],
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        };

        darray_push(queue_create_infos, queue_create_info);
    }

    VkDeviceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = next_device_features,
        .queueCreateInfoCount = darray_length(queue_create_infos),
        .pQueueCreateInfos = queue_create_infos,
        .pEnabledFeatures = &device_features,
        .enabledLayerCount =
            darray_length(surface->instance->validation_layers),
        .ppEnabledLayerNames = surface->instance->validation_layers,
        .enabledExtensionCount = darray_length(required_extensions),
        .ppEnabledExtensionNames = required_extensions,
    };

    Device self = {
        .physical_device = physical_device,
        .surface = surface,

        .graphics_family_index = graphics_family,
        .compute_family_index = compute_family,
        .present_family_index = present_family,
        .transfer_family_index = transfer_family,
    };

    vulkan_check(
        vkCreateDevice(physical_device, &create_info, NULL, &self.handle),
        "create logical device");

    vkGetDeviceQueue(self.handle, graphics_family, 0, &self.graphics_queue);
    vkGetDeviceQueue(self.handle, compute_family, 0, &self.compute_queue);
    vkGetDeviceQueue(self.handle, present_family, 0, &self.present_queue);
    vkGetDeviceQueue(self.handle, transfer_family, 0, &self.transfer_queue);

    return self;
}

void device_destroy(Device *device) {
    if (device->handle != NULL) {
        vkDestroyDevice(device->handle, NULL);
        device->handle = NULL;
    }
}

void device_wait_idle(const Device *device) {
    vkDeviceWaitIdle(device->handle);
}

static void check_required_extensions(VkPhysicalDevice device,
                                      darray(const char *)
                                          required_extensions) {
    u32 extension_count;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);
    VkExtensionProperties available_extensions[extension_count];
    vkEnumerateDeviceExtensionProperties(device,
                                         NULL,
                                         &extension_count,
                                         available_extensions);

    b8 missing_extension = false;
    for (u32 i = 0; i < darray_length(required_extensions); i++) {
        b8 found = false;
        for (u32 j = 0; j < extension_count; j++) {
            if (strncmp(available_extensions[j].extensionName,
                        required_extensions[i],
                        256) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            missing_extension = true;
            LOG_FATAL("missing required extension: %s", required_extensions[i]);
        }
    }

    if (missing_extension) {
        exit(EXIT_FAILURE);
    }
}

static u32 find_queue(VkQueueFamilyProperties *queue_families,
                      u32 queue_family_count,
                      const char *name,
                      VkQueueFlags required_bits,
                      VkQueueFlags excluded_bits) {
    for (u32 i = 0; i < queue_family_count; i++) {
        VkQueueFamilyProperties queue_family = queue_families[i];
        if (queue_family.queueCount > 0 &&
            queue_family.queueFlags & required_bits &&
            !(queue_family.queueFlags & excluded_bits))
            return i;
    }

    LOG_FATAL("found no matching %s queue", name);
    exit(EXIT_FAILURE);
}
