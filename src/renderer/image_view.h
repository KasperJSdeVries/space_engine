#ifndef IMAGE_VIEW_H
#define IMAGE_VIEW_H

#include "renderer/device.h"
#include "vulkan/vulkan_core.h"

typedef struct {
    const Device *device;
    VkImageView handle;
    VkImage image;
    VkFormat format;
} ImageView;

ImageView image_view_new(const Device *device,
                         VkImage image,
                         VkFormat format,
                         VkImageAspectFlags aspect_flags);
void image_view_destroy(ImageView *image_view);

#endif // IMAGE_VIEW_H
