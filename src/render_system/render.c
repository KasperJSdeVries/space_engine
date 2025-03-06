#include "render.h"

#include "render_system/device.h"
#include "render_system/instance.h"
#include "render_system/pipeline.h"
#include "render_system/swapchain.h"
#include "render_system/types.h"

static struct render_system_state state = {0};
static struct pipeline triangle_pipeline = {0};

b8 render_system_startup(const struct se_window *window) {
    if (!instance_create(&state.instance)) {
        return false;
    }

    if (!platform_surface_create(window, state.instance.handle, &state.surface)) {
        return false;
    }

    if (!device_create(&state.instance, state.surface, &state.device)) {
        return false;
    }

    if (!swapchain_create(&state.device, state.surface, window, &state.swapchain)) {
        return false;
    }

    if (!pipeline_create(&state.device, &triangle_pipeline)) {
        return false;
    }

    return true;
}

void render_system_shutdown(void) {
    pipeline_destroy(&state.device, &triangle_pipeline);
    swapchain_destroy(&state.device, &state.swapchain);
    device_destroy(&state.device);
    vkDestroySurfaceKHR(state.instance.handle, state.surface, NULL);
    instance_destroy(&state.instance);
}
