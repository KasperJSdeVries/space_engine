#ifndef APPLICATION_H
#define APPLICATION_H

#include "core/defines.h"
#include "renderer/command_buffers.h"
#include "renderer/command_pool.h"
#include "renderer/debug_utils_messenger.h"
#include "renderer/depth_buffer.h"
#include "renderer/device.h"
#include "renderer/fence.h"
#include "renderer/framebuffer.h"
#include "renderer/graphics_pipeline.h"
#include "renderer/instance.h"
#include "renderer/semaphore.h"
#include "renderer/surface.h"
#include "renderer/swapchain.h"
#include "renderer/window.h"

typedef struct {
    Window *window;
    Instance *instance;
    DebugUtilsMessenger *debug_utils_messenger;
    Surface *surface;
    Device *device;
    Swapchain *swapchain;
    darray(UniformBuffer) uniform_buffers;
    DepthBuffer *depth_buffer;
    GraphicsPipeline *graphics_pipeline;
    darray(Framebuffer) framebuffers;
    CommandPool *command_pool;
    CommandBuffers *command_buffers;
    darray(Semaphore) image_available_semaphores;
    darray(Semaphore) render_finished_semaphores;
    darray(Fence) in_flight_fences;
    Scene *scene;
    u64 current_frame;
    VkPresentModeKHR present_mode;
} Application;

Application application_new(WindowConfig window_config,
                            VkPresentModeKHR present_mode,
                            b8 enable_validation_layers);

void application_destroy(Application *application);

darray(VkPhysicalDevice) application_physical_devices(Application *application);
void application_set_physical_device(Application *application,
                                     VkPhysicalDevice physical_device);

void application_run(Application *application);

#endif // APPLICATION_H
