#include "sampler.h"

#include "renderer/vulkan.h"

Sampler sampler_new(const Device *device, SamplerConfig config) {
    Sampler self = {
        .device = device,
    };

    VkSamplerCreateInfo sampler_info = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = config.mag_filter,
        .minFilter = config.min_filter,
        .addressModeU = config.address_mode_u,
        .addressModeV = config.address_mode_v,
        .addressModeW = config.address_mode_w,
        .anisotropyEnable = config.anistropy_enable,
        .maxAnisotropy = config.max_anistropy,
        .borderColor = config.border_color,
        .unnormalizedCoordinates = config.unnormalized_coordinates,
        .compareEnable = config.compare_enable,
        .compareOp = config.compare_op,
        .mipmapMode = config.mipmap_mode,
        .mipLodBias = config.mip_lod_bias,
        .minLod = config.min_lod,
        .maxLod = config.max_lod,
    };

    vulkan_check(
        vkCreateSampler(device->handle, &sampler_info, NULL, &self.handle),
        "create sampler");

    return self;
}

void sampler_destroy(Sampler *self) {
    if (self->handle != NULL) {
        vkDestroySampler(self->device->handle, self->handle, NULL);
        self->handle = NULL;
    }
}
