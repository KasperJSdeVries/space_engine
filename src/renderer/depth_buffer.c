#include "depth_buffer.h"

#include "core/logging.h"
#include "renderer/device.h"
#include "renderer/device_memory.h"
#include "renderer/image.h"

static VkFormat find_supported_format(const Device *device,
                                      VkFormat *candidates,
                                      u32 candidate_count,
                                      VkImageTiling tiling,
                                      VkFormatFeatureFlags features);
static VkFormat find_depth_format(const Device *device);

DepthBuffer depth_buffer_new(CommandPool *command_pool, VkExtent2D extent) {
    DepthBuffer self = {
        .format = find_depth_format(command_pool->device),
    };

    const Device *device = command_pool->device;

    self.image = malloc(sizeof(Image));
    *self.image = image_new(device,
                            extent,
                            self.format,
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    self.image_memory = malloc(sizeof(DeviceMemory));
    *self.image_memory =
        image_allocate_memory(self.image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    self.image_view = malloc(sizeof(ImageView));
    *self.image_view = image_view_new(device,
                                      self.image->handle,
                                      self.format,
                                      VK_IMAGE_ASPECT_DEPTH_BIT);
    image_transition_layout(self.image,
                            command_pool,
                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    return self;
}

void depth_buffer_destroy(DepthBuffer *self) {
    image_view_destroy(self->image_view);
    free(self->image_view);
    image_destroy(self->image);
    free(self->image);
    device_memory_destroy(self->image_memory);
    free(self->image_memory);
}

static VkFormat find_supported_format(const Device *device,
                                      VkFormat *candidates,
                                      u32 candidate_count,
                                      VkImageTiling tiling,
                                      VkFormatFeatureFlags features) {
    for (u32 i = 0; i < candidate_count; i++) {
        VkFormat format = candidates[i];

        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(device->physical_device,
                                            format,
                                            &props);

        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features) {
            return format;
        }

        if (tiling == VK_IMAGE_TILING_OPTIMAL &&
            (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    LOG_FATAL("failed to find supported format");
    exit(EXIT_FAILURE);
}

static VkFormat find_depth_format(const Device *device) {
    return find_supported_format(
        device,
        (VkFormat[]){VK_FORMAT_D32_SFLOAT,
                     VK_FORMAT_D32_SFLOAT_S8_UINT,
                     VK_FORMAT_D24_UNORM_S8_UINT},
        3,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
