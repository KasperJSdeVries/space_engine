#ifndef APPLICATION_H
#define APPLICATION_H

#include "core/defines.h"
#include "renderer/bottom_level_acceleration_structure.h"
#include "renderer/command_buffers.h"
#include "renderer/command_pool.h"
#include "renderer/debug_utils_messenger.h"
#include "renderer/depth_buffer.h"
#include "renderer/device.h"
#include "renderer/fence.h"
#include "renderer/framebuffer.h"
#include "renderer/graphics_pipeline.h"
#include "renderer/instance.h"
#include "renderer/ray_tracing_pipeline.h"
#include "renderer/semaphore.h"
#include "renderer/shader_binding_table.h"
#include "renderer/surface.h"
#include "renderer/swapchain.h"
#include "renderer/top_level_acceleration_structure.h"
#include "renderer/window.h"

typedef struct {
    Window *window;
    Instance instance;
    DebugUtilsMessenger *debug_utils_messenger;
    Surface surface;
    Device *device;
    Swapchain swapchain;
    darray(UniformBuffer) uniform_buffers;
    DepthBuffer depth_buffer;
    GraphicsPipeline *graphics_pipeline;
    darray(Framebuffer) framebuffers;
    CommandPool command_pool;
    CommandBuffers command_buffers;
    darray(Semaphore) image_available_semaphores;
    darray(Semaphore) render_finished_semaphores;
    darray(Fence) in_flight_fences;
    darray(BottomLevelAccelerationStructure) bottom_as;
    Buffer bottom_buffer;
    DeviceMemory bottom_buffer_memory;
    TopLevelAccelerationStructure *top_as;
    Buffer instances_buffer;
    DeviceMemory instances_buffer_memory;
    Buffer top_buffer;
    DeviceMemory top_buffer_memory;
    Image accumulation_image;
    DeviceMemory accumulation_image_memory;
    ImageView accumulation_image_view;
    Image output_image;
    DeviceMemory output_image_memory;
    ImageView output_image_view;
    RayTracingPipeline ray_tracing_pipeline;
    ShaderBindingTable shader_binding_table;
    Scene *scene;
    u64 current_frame;
    VkPresentModeKHR present_mode;
    u32 total_number_of_samples;
    u32 number_of_samples;
} Application;

Application application_new(WindowConfig window_config,
                            Scene *scene,
                            VkPresentModeKHR present_mode,
                            b8 enable_validation_layers);
void application_destroy(Application *application);

void application_run(Application *application);

#endif // APPLICATION_H
