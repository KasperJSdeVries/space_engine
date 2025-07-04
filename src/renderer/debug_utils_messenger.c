#include "debug_utils_messenger.h"

#include "containers/darray.h"
#include "core/defines.h"
#include "core/logging.h"
#include "renderer/instance.h"
#include "renderer/vulkan.h"

#include <vulkan/vulkan.h>

struct DebugUtilsMessenger {
    const Instance *instance;
    VkDebugUtilsMessageSeverityFlagBitsEXT threshold;
    VkDebugUtilsMessengerEXT messenger;
};

static const char *object_type_to_string(const VkObjectType object_type);
VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(
    const VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    const VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT *const callback_data,
    void *const user_data);
static VkResult createDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pMessenger);
static void destroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT messenger,
    const VkAllocationCallbacks *pAllocator);

DebugUtilsMessenger *debug_utils_messenger_new(
    const Instance *instance,
    VkDebugUtilsMessageSeverityFlagBitsEXT threshold) {
    if (darray_length(instance->validation_layers) == 0) {
        return NULL;
    }

    VkDebugUtilsMessageSeverityFlagBitsEXT severity = 0;

    switch (threshold) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        severity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
        fallthrough;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        severity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
        fallthrough;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        severity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        fallthrough;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        severity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        break;
    default:
        LOG_FATAL("invalid threshold");
        exit(EXIT_FAILURE);
    }

    VkDebugUtilsMessengerCreateInfoEXT create_info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = severity,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = vulkan_debug_callback,
        .pUserData = NULL,
    };

    DebugUtilsMessenger *this = malloc(sizeof(*this));

    this->threshold = threshold;
    this->instance = instance;

    vulkan_check(createDebugUtilsMessengerEXT(instance->handle,
                                              &create_info,
                                              NULL,
                                              &this->messenger),
                 "set up Vulkan debug callback");

    return this;
}

void debug_utils_messenger_destroy(DebugUtilsMessenger *this) {
    if (this->messenger != NULL) {
        destroyDebugUtilsMessengerEXT(this->instance->handle,
                                      this->messenger,
                                      NULL);
        this->messenger = NULL;
    }
    free(this);
}

static const char *object_type_to_string(const VkObjectType object_type) {
    switch (object_type) {
#define STR(e)                                                                 \
    case VK_OBJECT_TYPE_##e:                                                   \
        return #e
        STR(UNKNOWN);
        STR(INSTANCE);
        STR(PHYSICAL_DEVICE);
        STR(DEVICE);
        STR(QUEUE);
        STR(SEMAPHORE);
        STR(COMMAND_BUFFER);
        STR(FENCE);
        STR(DEVICE_MEMORY);
        STR(BUFFER);
        STR(IMAGE);
        STR(EVENT);
        STR(QUERY_POOL);
        STR(BUFFER_VIEW);
        STR(IMAGE_VIEW);
        STR(SHADER_MODULE);
        STR(PIPELINE_CACHE);
        STR(PIPELINE_LAYOUT);
        STR(RENDER_PASS);
        STR(PIPELINE);
        STR(DESCRIPTOR_SET_LAYOUT);
        STR(SAMPLER);
        STR(DESCRIPTOR_POOL);
        STR(DESCRIPTOR_SET);
        STR(FRAMEBUFFER);
        STR(COMMAND_POOL);
        STR(SAMPLER_YCBCR_CONVERSION);
        STR(DESCRIPTOR_UPDATE_TEMPLATE);
        STR(SURFACE_KHR);
        STR(SWAPCHAIN_KHR);
        STR(DISPLAY_KHR);
        STR(DISPLAY_MODE_KHR);
        STR(DEBUG_REPORT_CALLBACK_EXT);
        STR(DEBUG_UTILS_MESSENGER_EXT);
        STR(ACCELERATION_STRUCTURE_KHR);
        STR(VALIDATION_CACHE_EXT);
        STR(PERFORMANCE_CONFIGURATION_INTEL);
        STR(DEFERRED_OPERATION_KHR);
        STR(INDIRECT_COMMANDS_LAYOUT_NV);
#undef STR
    default:
        return "unknown";
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(
    const VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    const VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT *const callback_data,
    void *const user_data) {
    UNUSED(user_data);

    LogLevel level;
    switch (message_severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        level = LOG_LEVEL_TRACE;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        level = LOG_LEVEL_INFO;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        level = LOG_LEVEL_WARN;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        level = LOG_LEVEL_ERROR;
        break;
    default:
        level = LOG_LEVEL_DEBUG;
        break;
    }

    char *message_type_string;
    switch (message_type) {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
        message_type_string = "GENERAL";
        break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
        message_type_string = "VALIDATION";
        break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
        message_type_string = "PERFORMANCE";
        break;
    default:
        message_type_string = "UNKNOWN";
        break;
    }

    LOG(level, "%s: %s", message_type_string, callback_data->pMessage);

    if (callback_data->objectCount > 0 &&
        message_severity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        LOG(level, " ");
        LOG(level, "  Objects (%d):", callback_data->objectCount);

        for (u32 i = 0; i < callback_data->objectCount; i++) {
            VkDebugUtilsObjectNameInfoEXT object = callback_data->pObjects[i];
            LOG(level,
                "  - Object[%d]: Type: %s, Handle: %p, Name: '%s'",
                i,
                object_type_to_string(object.objectType),
                (void *)object.objectHandle,
                object.pObjectName == NULL ? object.pObjectName : "");
        }
        LOG(level, " ");
    }

    return VK_FALSE;
}

static VkResult createDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
        return func(instance, pCreateInfo, pAllocator, pMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static void destroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT messenger,
    const VkAllocationCallbacks *pAllocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL) {
        func(instance, messenger, pAllocator);
    }
}
