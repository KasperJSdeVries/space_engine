#ifndef RENDER_DEVICE_H
#define RENDER_DEVICE_H

#include "containers/darray.h"
#include "renderer/surface.h"
#include "vulkan.h"

typedef struct {
    VkPhysicalDevice physical_device;
    const Surface *surface;
    VkDevice handle;
    u32 graphics_family_index;
    u32 compute_family_index;
    u32 present_family_index;
    u32 transfer_family_index;
    VkQueue graphics_queue;
    VkQueue compute_queue;
    VkQueue present_queue;
    VkQueue transfer_queue;
} Device;

Device device_new(VkPhysicalDevice physical_device,
                  const Surface *surface,
                  darray(const char *) required_extensions,
                  VkPhysicalDeviceFeatures device_features,
                  const void *next_device_features);
void device_destroy(Device *device);

void device_wait_idle(const Device *device);

#endif // RENDER_DEVICE_H
