#include "image.h"

#include "types.h"

#include "core/assert.h"
#include "vulkan/vulkan_core.h"

b8 image_view_create(const struct device *device,
                     VkImage image,
                     VkFormat format,
                     VkImageView *image_view) {
    VkImageViewCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components =
            {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };

    if (!ASSERT(vkCreateImageView(device->handle, &create_info, NULL, image_view) == VK_SUCCESS)) {
        return false;
    }
    return true;
}
