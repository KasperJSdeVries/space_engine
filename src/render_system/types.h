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
	VkImage *images;
	VkImageView *image_views;
	VkFramebuffer *framebuffers;
	u32 image_count;
	VkFormat format;
	VkExtent2D extent;
};

struct renderpass {
	VkRenderPass handle;
};

struct pipeline {
	VkPipeline handle;
	VkPipelineLayout layout;
};

struct commandpool {
	VkCommandPool handle;
};

struct commandbuffer {
	VkCommandBuffer handle;
};

struct render_system_state {
    struct instance instance;
    struct device device;
    VkSurfaceKHR surface;
    struct swapchain swapchain;
	struct renderpass renderpass;
	struct commandpool commandpool;
	struct commandbuffer commandbuffer;
	VkSemaphore image_available_semaphore;
	VkSemaphore render_finished_semaphore;
	VkFence in_flight_fence;
};

#endif // RENDER_TYPES_H
