#include "core/defines.h"
#include "core/logging.h"
#include "renderer/application.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void set_vulkan_device(Application *application);

int main(void) {
    WindowConfig window_config = {
        .title = "Vulkan Window",
        .width = 1080,
        .height = 720,
        .cursor_disabled = false,
        .fullscreen = false,
        .resizable = true,
    };

    Application application =
        application_new(window_config, VK_PRESENT_MODE_IMMEDIATE_KHR, true);

    set_vulkan_device(&application);

    application_run(&application);

    application_destroy(&application);
}

static void set_vulkan_device(Application *application) {
    darray(VkPhysicalDevice) physical_devices =
        application_physical_devices(application);

    VkPhysicalDevice found_device = VK_NULL_HANDLE;
    for (u32 i = 0; i < darray_length(physical_devices); i++) {
        VkPhysicalDevice device = physical_devices[i];

        VkPhysicalDeviceProperties2 props = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        };

        vkGetPhysicalDeviceProperties2(device, &props);

        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceFeatures(device, &device_features);

        if (!device_features.geometryShader) {
            continue;
        }

        u32 extension_count;
        vkEnumerateDeviceExtensionProperties(device,
                                             NULL,
                                             &extension_count,
                                             NULL);
        VkExtensionProperties extensions[extension_count];
        vkEnumerateDeviceExtensionProperties(device,
                                             NULL,
                                             &extension_count,
                                             extensions);

        b8 has_ray_tracing = false;
        for (u32 j = 0; j < extension_count; j++) {
            if (strcmp(extensions[j].extensionName,
                       VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0) {
                has_ray_tracing = true;
                break;
            }
        }
        if (!has_ray_tracing) {
            continue;
        }

        u32 queue_family_count;
        vkGetPhysicalDeviceQueueFamilyProperties(device,
                                                 &queue_family_count,
                                                 NULL);
        VkQueueFamilyProperties queue_families[queue_family_count];
        vkGetPhysicalDeviceQueueFamilyProperties(device,
                                                 &queue_family_count,
                                                 queue_families);

        b8 has_graphics_queue = false;
        for (u32 j = 0; j < queue_family_count; j++) {
            if (queue_families[j].queueCount > 0 &&
                queue_families[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                has_graphics_queue = true;
                break;
            }
        }
        if (!has_graphics_queue) {
            continue;
        }

        found_device = device;
        break;
    }

    if (found_device == VK_NULL_HANDLE) {
        LOG_FATAL("cannot find a suitable device");
        exit(EXIT_FAILURE);
    }

    application_set_physical_device(application, found_device);
}
