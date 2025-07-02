#ifndef VULKAN_H
#define VULKAN_H

#define NOMINMAX
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#undef APIENTRY

void vulkan_check(VkResult result, const char *operiation);
const char *vulkan_result_to_string(VkResult result);

#endif // VULKAN_H
