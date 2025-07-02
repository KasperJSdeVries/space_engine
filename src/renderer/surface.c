#include "surface.h"

#include "renderer/vulkan.h"

Surface surface_new(const Instance *instance) {
    Surface surface = {
        .instance = instance,
    };

    vulkan_check(glfwCreateWindowSurface(instance->handle,
                                         window_handle(instance->window),
                                         NULL,
                                         &surface.handle),
                 "create window surface");

    return surface;
}

void surface_destroy(Surface *surface) {
    if (surface->handle != NULL) {
        vkDestroySurfaceKHR(surface->instance->handle, surface->handle, NULL);
        surface->handle = NULL;
    }
}
