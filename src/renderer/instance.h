#ifndef RENDER_INSTANCE_H
#define RENDER_INSTANCE_H

#include "containers/darray.h"
#include "renderer/window.h"

typedef struct {
    VkInstance handle;
    const Window *window;
    darray(const char *) validation_layers;
} Instance;

Instance instance_new(const Window *window,
                      darray(const char *) validation_layers,
                      u32 vulkan_version);
void instance_destroy(Instance *instance);

darray(VkExtensionProperties) instance_extensions(void);
darray(VkLayerProperties) instance_layers(void);
darray(VkPhysicalDevice) instance_physical_devices(const Instance *this);

#endif // RENDER_INSTANCE_H
