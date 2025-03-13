#ifndef RENDER_TYPES_H
#define RENDER_TYPES_H

#include "core/defines.h"

#include <vulkan/vulkan.h>

#if SE_DEBUG
#define ENABLE_VALIDATION_LAYERS true
#else
#define ENABLE_VALIDATION_LAYERS false
#endif

#define MAX_FRAMES_IN_FLIGHT 3

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
	struct commandbuffer commandbuffers[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore image_available_semaphores[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore render_finished_semaphores[MAX_FRAMES_IN_FLIGHT];
	VkFence in_flight_fences[MAX_FRAMES_IN_FLIGHT];
    u32 image_index;
	u32 current_frame;
	b8 framebuffer_resized;
};

#endif // RENDER_TYPES_H
