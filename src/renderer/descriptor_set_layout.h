#ifndef DESCRIPTOR_SET_LAYOUT_H
#define DESCRIPTOR_SET_LAYOUT_H

#include "renderer/device.h"

typedef struct {
    VkDescriptorSetLayout handle;
    const Device *device;
} DescriptorSetLayout;

DescriptorSetLayout descriptor_set_layout_new(
    const Device *device /* TODO: bindings */);
void descriptor_set_layout_destroy(DescriptorSetLayout *self);

#endif // DESCRIPTOR_SET_LAYOUT_H
