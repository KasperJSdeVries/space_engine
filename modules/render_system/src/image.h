#ifndef RENDER_IMAGE_H
#define RENDER_IMAGE_H

#include "types.h"

b8 image_view_create(const struct device *device,
                       VkImage image,
                       VkFormat format,
                       VkImageView *image_view);

#endif // RENDER_IMAGE_H
