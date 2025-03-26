#include "instance.h"

#include "core/assert.h"
#include "platform/platform.h"

#if ENABLE_VALIDATION_LAYERS
static const char *instance_extensions[] = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
static const u32 instance_extension_count =
    sizeof(instance_extensions) / sizeof(*instance_extensions);

static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                         VkDebugUtilsMessageTypeFlagsEXT message_type,
                         const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                         void *user_data);
#endif

b8 instance_create(struct instance *instance) {
    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "space game",
        .applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
        .pEngineName = "space engine",
        .engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
        .apiVersion = VK_API_VERSION_1_3,
    };

#if ENABLE_VALIDATION_LAYERS
    VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
        .pfnUserCallback = debug_messenger_callback,
    };
#endif // ENABLE_VALIDATION_LAYERS

    u32 required_extension_count;
    platform_get_required_extensions(&required_extension_count, NULL);

#if ENABLE_VALIDATION_LAYERS
    const char *extensions[required_extension_count + instance_extension_count];
#else
    const char *extensions[required_extension_count];
#endif

    platform_get_required_extensions(&required_extension_count, extensions);

#if ENABLE_VALIDATION_LAYERS
    for (u32 i = 0; i < instance_extension_count; i++) {
        extensions[required_extension_count + i] = instance_extensions[i];
    }
#endif

    VkInstanceCreateInfo instance_create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .ppEnabledExtensionNames = extensions,
#if ENABLE_VALIDATION_LAYERS
        .enabledExtensionCount = required_extension_count + instance_extension_count,
        .ppEnabledLayerNames = validation_layers,
        .enabledLayerCount = validation_layer_count,
        .pNext = &debug_messenger_create_info,
#else
        .enabledExtensionCount = required_extension_count,
#endif // ENABLE_VALIDATION_LAYERS
    };

    if (!(ASSERT(vkCreateInstance(&instance_create_info, NULL, &instance->handle) == VK_SUCCESS))) {
        return false;
    }

#if ENABLE_VALIDATION_LAYERS
    if (!(ASSERT(vkCreateDebugUtilsMessengerEXT(instance->handle,
                                                &debug_messenger_create_info,
                                                NULL,
                                                &instance->debug_messenger) == VK_SUCCESS))) {
        return false;
    }
#endif // ENABLE_VALIDATION_LAYERS

    return true;
}

void instance_destroy(struct instance *instance) {
#if ENABLE_VALIDATION_LAYERS
    vkDestroyDebugUtilsMessengerEXT(instance->handle, instance->debug_messenger, NULL);
#endif // ENABLE_VALIDATION_LAYERS
    vkDestroyInstance(instance->handle, NULL);
}

#if ENABLE_VALIDATION_LAYERS
static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                         VkDebugUtilsMessageTypeFlagsEXT message_type,
                         const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                         void *user_data) {
    UNUSED(user_data);
    UNUSED(message_type);

    switch (severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        LOG_ERROR("%s", callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        LOG_WARN("%s", callback_data->pMessage);
        break;
    default:
        LOG_INFO("%s", callback_data->pMessage);
        break;
    };

    return VK_FALSE;
}

VkResult vkCreateDebugUtilsMessengerEXT(VkInstance instance,
                                        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator,
                                        VkDebugUtilsMessengerEXT *pMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT function =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
                                                                  "vkCreateDebugUtilsMessengerEXT");
    if (function != NULL) {
        return function(instance, pCreateInfo, pAllocator, pMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void vkDestroyDebugUtilsMessengerEXT(VkInstance instance,
                                     VkDebugUtilsMessengerEXT messenger,
                                     const VkAllocationCallbacks *pAllocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT function = (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (function != NULL) {
        function(instance, messenger, pAllocator);
    }
}
#endif // ENABLE_VALIDATION_LAYERS
