#ifndef RENDER_TYPES_H
#define RENDER_TYPES_H

#include "core/darray.h"
#include "core/defines.h"
#include "vulkan/vulkan_core.h"

#define CGLM_OMIT_NS_FROM_STRUCT_API
#include <cglm/struct.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <vulkan/vulkan.h>

#define ENABLE_VALIDATION_LAYERS true

#define MAX_FRAMES_IN_FLIGHT 3

struct uniform_buffer_object {
    mat4s model;
    mat4s view;
    mat4s projection;
};

struct instance {
    VkInstance handle;
#if ENABLE_VALIDATION_LAYERS
    VkDebugUtilsMessengerEXT debug_messenger;
#endif
};

struct window {
    Display *display; // reference to display the window was created with
    Window handle;
    Atom wm_delete_window;
};

struct surface {
    VkSurfaceKHR handle;
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

struct renderbuffer {
    VkBuffer handle;
    VkDeviceMemory memory;
    u64 size;
};

struct renderpass {
    VkRenderPass handle;
};

typedef struct {
    const struct renderer *renderer;

    VkShaderModule vertex_shader_module;
    VkShaderModule fragment_shader_module;
    VkPipelineShaderStageCreateInfo shader_stages[2];

    darray(VkVertexInputBindingDescription) vertex_input_bindings;
    darray(VkVertexInputAttributeDescription) vertex_input_attributes;

    u64 ubo_size;
    VkCullModeFlags cull_mode;
    VkPrimitiveTopology topology;
    b8 enable_alpha_blending;

    darray(VkPushConstantRange) push_constant_ranges;
} pipeline_builder;

struct pipeline {
    VkPipeline handle;
    VkPipelineLayout layout;
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet descriptor_sets[MAX_FRAMES_IN_FLIGHT];
    struct renderbuffer uniform_buffer;
    void *uniform_buffer_mapped;
};

struct commandpool {
    VkCommandPool handle;
};

struct commandbuffer {
    VkCommandBuffer handle;
};

struct renderer {
    struct instance instance;
    struct window window;
    struct device device;
    struct surface surface;
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
