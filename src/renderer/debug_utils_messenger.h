#ifndef DEBUG_UTILS_MESSENGER_H
#define DEBUG_UTILS_MESSENGER_H

#include "renderer/instance.h"

#include <vulkan/vulkan.h>

typedef struct DebugUtilsMessenger DebugUtilsMessenger;

DebugUtilsMessenger *debug_utils_messenger_new(
    const Instance *instance,
    VkDebugUtilsMessageSeverityFlagBitsEXT threshold);
void debug_utils_messenger_destroy(DebugUtilsMessenger *debug_utils_messenger);

#endif // DEBUG_UTILS_MESSENGER_H
