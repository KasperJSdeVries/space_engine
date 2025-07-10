#ifndef VULKAN_H
#define VULKAN_H

#define NOMINMAX
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#undef APIENTRY

#include "core/logging.h"

#define vulkan_check(result, operation)                                        \
    do {                                                                       \
        if (result != VK_SUCCESS) {                                            \
            LOG_FATAL("failed to %s (%s)",                                     \
                      operation,                                               \
                      vulkan_result_to_string(result));                        \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
    } while (0)

const char *vulkan_result_to_string(VkResult result);

#endif // VULKAN_H
