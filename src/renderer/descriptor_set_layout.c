#include "descriptor_set_layout.h"

#include "core/logging.h"

DescriptorSetLayout descriptor_set_layout_new(
    const Device *device /* TODO: bindings */) {
    DescriptorSetLayout self = {
        .device = device,
    };

    VkDescriptorSetLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    };

    TODO("bindings");

    vulkan_check(vkCreateDescriptorSetLayout(device->handle,
                                             &layout_info,
                                             NULL,
                                             &self.handle),
                 "create descriptor set layout");

    return self;
}

void descriptor_set_layout_destroy(DescriptorSetLayout *self) {
    if (self->handle != NULL) {
        vkDestroyDescriptorSetLayout(self->device->handle, self->handle, NULL);
        self->handle = NULL;
    }
}
