#ifndef RENDER_TYPES_H
#define RENDER_TYPES_H

#include "core/defines.h"

#include <vulkan/vulkan.h>

#ifdef SE_DEBUG
#define ENABLE_VALIDATION_LAYERS true
#else
#define ENABLE_VALIDATION_LAYERS false
#endif

struct instance {
    VkInstance handle;
#if ENABLE_VALIDATION_LAYERS
    VkDebugUtilsMessengerEXT debug_messenger;
#endif
};

struct device {
    VkPhysicalDevice physical_device;
    VkDevice handle;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkQueue transfer_queue;
    VkQueue compute_queue;
};

struct swapchain {
    VkSwapchainKHR handle;
	u32 image_count;
	VkImage *images;
	VkImageView *image_views;
};

struct render_system_state {
    struct instance instance;
    struct device device;
    VkSurfaceKHR surface;
    struct swapchain swapchain;
};

#endif // RENDER_TYPES_H
