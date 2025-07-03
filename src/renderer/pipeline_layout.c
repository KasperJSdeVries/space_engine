#include "pipeline_layout.h"
#include "core/defines.h"
#include "vulkan/vulkan_core.h"

PipelineLayout pipeline_layout_new(
    const Device *device,
    VkDescriptorSetLayout descriptor_set_layout) {
    PipelineLayout self = {
        .device = device,
    };

    VkDescriptorSetLayout descriptor_set_layouts[] = {
        descriptor_set_layout,
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = ARRAY_SIZE(descriptor_set_layouts),
        .pSetLayouts = descriptor_set_layouts,
    };

    vulkan_check(vkCreatePipelineLayout(device->handle,
                                        &pipeline_layout_info,
                                        NULL,
                                        &self.handle),
                 "create pipeline layout");

    return self;
}

void pipeline_layout_destroy(PipelineLayout *self) {
    if (self->handle != NULL) {
        vkDestroyPipelineLayout(self->device->handle, self->handle, NULL);
        self->handle = NULL;
    }
}
