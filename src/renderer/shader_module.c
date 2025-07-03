#include "shader_module.h"
#include "assets/file.h"
#include "vulkan/vulkan_core.h"

ShaderModule shader_module_new(const Device *device, const char *filename) {
    ShaderModule self = {
        .device = device,
    };

    u64 buffer_size;
    u32 *buffer = (u32 *)file_read(filename, &buffer_size);

    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = buffer_size,
        .pCode = buffer,
    };

    vulkan_check(
        vkCreateShaderModule(device->handle, &create_info, NULL, &self.handle),
        "create shader module");

    free(buffer);

    return self;
}

void shader_module_destroy(ShaderModule *self) {
    if (self->handle != NULL) {
        vkDestroyShaderModule(self->device->handle, self->handle, NULL);
        self->handle = NULL;
    }
}

VkPipelineShaderStageCreateInfo shader_module_create_shader_stage(
    const ShaderModule *self,
    VkShaderStageFlagBits stage) {
    return (VkPipelineShaderStageCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = stage,
        .module = self->handle,
        .pName = "main",
    };
}
