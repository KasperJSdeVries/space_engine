#ifndef SAMPLER_H
#define SAMPLER_H

#include "renderer/device.h"
#include "vulkan/vulkan_core.h"

typedef struct {
    VkFilter mag_filter;
    VkFilter min_filter;
    VkSamplerAddressMode address_mode_u;
    VkSamplerAddressMode address_mode_v;
    VkSamplerAddressMode address_mode_w;
    b8 anistropy_enable;
    f32 max_anistropy;
    VkBorderColor border_color;
    b8 unnormalized_coordinates;
    b8 compare_enable;
    VkCompareOp compare_op;
    VkSamplerMipmapMode mipmap_mode;
    f32 mip_lod_bias;
    f32 min_lod;
    f32 max_lod;
} SamplerConfig;

static inline SamplerConfig sampler_config_default(void) {
    return (SamplerConfig){
        .min_filter = VK_FILTER_LINEAR,
        .mag_filter = VK_FILTER_LINEAR,
        .address_mode_u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .address_mode_v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .address_mode_w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .anistropy_enable = true,
        .max_anistropy = 16.0f,
        .border_color = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .compare_op = VK_COMPARE_OP_ALWAYS,
        .mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
    };
}

typedef struct {
    const Device *device;
    VkSampler handle;
} Sampler;

Sampler sampler_new(const Device *device, SamplerConfig config);
void sampler_destroy(Sampler *self);

#endif // SAMPLER_H
