#include "instance.h"

#include "containers/darray.h"
#include "core/defines.h"
#include "core/logging.h"
#include "renderer/vulkan.h"
#include "renderer/window.h"
#include "vulkan/vulkan_core.h"

#include <stdlib.h>
#include <string.h>

static void check_vulkan_minimum_version(u32 min_version);
static void check_vulkan_validation_layer_support(darray(const char *)
                                                      validation_layers);

Instance instance_new(const Window *window,
                      darray(const char *) validation_layers,
                      u32 vulkan_version) {
    Instance self = {
        .window = window,
        .validation_layers = validation_layers,
    };

    check_vulkan_minimum_version(vulkan_version);

    darray(const char *) extensions = window_get_required_instance_extensions();

    check_vulkan_validation_layer_support(validation_layers);

    if (darray_length(validation_layers) > 0) {
        darray_push(extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "SpaceGame",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = vulkan_version,
    };

    VkInstanceCreateInfo instance_create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = darray_length(extensions),
        .ppEnabledExtensionNames = extensions,
        .enabledLayerCount = darray_length(validation_layers),
        .ppEnabledLayerNames = validation_layers,
    };

    vulkan_check(vkCreateInstance(&instance_create_info, NULL, &self.handle),
                 "create instance");

    darray_destroy(extensions);

    return self;
}

void instance_destroy(Instance *instance) {
    if (instance->handle != NULL) {
        vkDestroyInstance(instance->handle, NULL);
        instance->handle = NULL;
    }

    darray_destroy(instance->validation_layers);
}

darray(VkExtensionProperties) instance_extensions(void) {
    darray(VkExtensionProperties) da = darray_new(VkExtensionProperties);

    u32 extension_count;
    vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);
    VkExtensionProperties extensions[extension_count];
    vkEnumerateInstanceExtensionProperties(NULL, &extension_count, extensions);

    for (u32 i = 0; i < extension_count; i++) {
        darray_push(da, extensions[i]);
    }

    return da;
}

darray(VkLayerProperties) instance_layers(void) {
    darray(VkLayerProperties) da = darray_new(VkLayerProperties);

    u32 layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, NULL);
    VkLayerProperties layers[layer_count];
    vkEnumerateInstanceLayerProperties(&layer_count, layers);

    for (u32 i = 0; i < layer_count; i++) {
        darray_push(da, layers[i]);
    }

    return da;
}

darray(VkPhysicalDevice) instance_physical_devices(const Instance *this) {
    darray(VkPhysicalDevice) da = darray_new(VkPhysicalDevice);

    u32 device_count;
    vkEnumeratePhysicalDevices(this->handle, &device_count, NULL);
    VkPhysicalDevice physical_devices[device_count];
    vkEnumeratePhysicalDevices(this->handle, &device_count, physical_devices);

    for (u32 i = 0; i < device_count; i++) {
        darray_push(da, physical_devices[i]);
    }

    if (darray_length(da) == 0) {
        LOG_FATAL("found no Vulkan physical devices");

        exit(EXIT_FAILURE);
    }

    return da;
}

static void check_vulkan_minimum_version(u32 min_version) {
    u32 version;
    vulkan_check(vkEnumerateInstanceVersion(&version),
                 "query instance version");

    if (min_version > version) {
        LOG_FATAL("minimum required version not found (required: %d.%d.%d, "
                  "found: %d.%d.%d)",
                  VK_VERSION_MAJOR(min_version),
                  VK_VERSION_MINOR(min_version),
                  VK_VERSION_PATCH(min_version),
                  VK_VERSION_MAJOR(version),
                  VK_VERSION_MINOR(version),
                  VK_VERSION_PATCH(version));

        exit(EXIT_FAILURE);
    }
}

static void check_vulkan_validation_layer_support(darray(const char *)
                                                      validation_layers) {
    u32 available_layer_count;
    vkEnumerateInstanceLayerProperties(&available_layer_count, NULL);
    VkLayerProperties available_layers[available_layer_count];
    vkEnumerateInstanceLayerProperties(&available_layer_count,
                                       available_layers);

    for (u32 i = 0; i < darray_length(validation_layers); i++) {
        b8 found = false;
        for (u32 j = 0; j < available_layer_count; j++) {
            if (strcmp(validation_layers[i], available_layers[j].layerName) ==
                0) {
                found = true;
                break;
            }
        }
        if (!found) {
            LOG_FATAL("could not find the requested validation layer: '%s'",
                      validation_layers[i]);
            exit(EXIT_FAILURE);
        }
    }
}
