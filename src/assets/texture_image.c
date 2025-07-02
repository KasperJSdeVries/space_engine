#include "texture_image.h"
#include "renderer/buffer.h"
#include "renderer/image.h"
#include "renderer/image_view.h"
#include "vulkan/vulkan_core.h"

TextureImage texture_image_new(CommandPool *command_pool,
                               const Texture *texture) {
    VkDeviceSize image_size = texture->width * texture->height * 4;
    const Device *device = command_pool->device;

    Buffer staging_buffer =
        buffer_new(device, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    DeviceMemory staging_buffer_memory =
        buffer_allocate_memory(&staging_buffer,
                               0,
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    void *data = device_memory_map(&staging_buffer_memory, 0, image_size);
    memcpy(data, texture->pixels, image_size);
    device_memory_unmap(&staging_buffer_memory);

    TextureImage self = {0};
    self.image = image_new(
        device,
        (VkExtent2D){.width = texture->width, .height = texture->height},
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    self.image_memory =
        image_allocate_memory(&self.image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    self.image_view = image_view_new(device,
                                     self.image.handle,
                                     self.image.format,
                                     VK_IMAGE_ASPECT_COLOR_BIT);
    self.sampler = sampler_new(device, sampler_config_default());

    image_transition_layout(&self.image,
                            command_pool,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    image_copy_from(&self.image, command_pool, &staging_buffer);
    image_transition_layout(&self.image,
                            command_pool,
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    buffer_destroy(&staging_buffer);
    device_memory_destroy(&staging_buffer_memory);

    return self;
}

void texture_image_destroy(TextureImage *self) {
    sampler_destroy(&self->sampler);
    image_view_destroy(&self->image_view);
    image_destroy(&self->image);
    device_memory_destroy(&self->image_memory);
}
