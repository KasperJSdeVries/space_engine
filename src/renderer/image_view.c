#include "image_view.h"
#include "vulkan/vulkan_core.h"

ImageView image_view_new(const Device *device,
                         VkImage image,
                         VkFormat format,
                         VkImageAspectFlags aspect_flags) {
    ImageView self = {
        .device = device,
        .image = image,
        .format = format,
    };

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
                .aspectMask = aspect_flags,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };

    vulkan_check(vkCreateImageView(self.device->handle,
                                   &create_info,
                                   NULL,
                                   &self.handle),
                 "create image view");

    return self;
}

void image_view_destroy(ImageView *self) {
    if (self->handle != NULL) {
        vkDestroyImageView(self->device->handle, self->handle, NULL);
        self->handle = NULL;
    }
}
