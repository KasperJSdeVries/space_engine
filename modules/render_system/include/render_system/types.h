#ifndef RENDER_TYPES_H
#define RENDER_TYPES_H

#include "core/defines.h"
#include "math/mat4.h"

#include <vulkan/vulkan.h>

#define ENABLE_VALIDATION_LAYERS true

#define MAX_FRAMES_IN_FLIGHT 3

struct uniform_buffer_object {
    mat4 model;
    mat4 view;
    mat4 projection;
};

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
    VkDescriptorSetLayout descriptor_set_layout;
	VkDescriptorPool descriptor_pool;
	VkDescriptorSet descriptor_sets[MAX_FRAMES_IN_FLIGHT];
};

struct commandpool {
    VkCommandPool handle;
};

struct commandbuffer {
    VkCommandBuffer handle;
};

struct renderbuffer {
    VkBuffer handle;
    VkDeviceMemory memory;
    u64 size;
};

struct renderer {
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
    struct renderbuffer ubo_buffers[MAX_FRAMES_IN_FLIGHT];
    void *ubo_buffers_mapped[MAX_FRAMES_IN_FLIGHT];
    u32 image_index;
    u32 current_frame;
    b8 framebuffer_resized;
};

#endif // RENDER_TYPES_H
