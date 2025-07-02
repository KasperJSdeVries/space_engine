#ifndef DEPTH_BUFFER_H
#define DEPTH_BUFFER_H

#include "renderer/command_pool.h"
#include "renderer/device_memory.h"
#include "renderer/image.h"
#include "renderer/image_view.h"

typedef struct {
    VkFormat format;
    Image *image;
    DeviceMemory *image_memory;
    ImageView *image_view;
} DepthBuffer;

DepthBuffer depth_buffer_new(CommandPool *command_pool, VkExtent2D extent);
void depth_buffer_destroy(DepthBuffer *self);

static inline b8 depth_buffer_has_stencil_component(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
           format == VK_FORMAT_D24_UNORM_S8_UINT;
}

#endif // DEPTH_BUFFER_H
