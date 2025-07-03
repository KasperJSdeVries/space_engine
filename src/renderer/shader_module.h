#ifndef SHADER_MODULE_H
#define SHADER_MODULE_H

#include "renderer/device.h"

typedef struct {
    VkShaderModule handle;
    const Device *device;
} ShaderModule;

ShaderModule shader_module_new(const Device *device, const char *filename);
void shader_module_destroy(ShaderModule *self);

VkPipelineShaderStageCreateInfo shader_module_create_shader_stage(
    const ShaderModule *self,
    VkShaderStageFlagBits stage);

#endif // SHADER_MODULE_H
