#ifndef PIPELINE_LAYOUT_H
#define PIPELINE_LAYOUT_H

#include "renderer/descriptor_set_layout.h"
#include "renderer/device.h"

typedef struct {
    VkPipelineLayout handle;
    const Device *device;
} PipelineLayout;

PipelineLayout pipeline_layout_new(
    const Device *device,
    const DescriptorSetLayout *descriptor_set_layout);
void pipeline_layout_destroy(PipelineLayout *self);

#endif // PIPELINE_LAYOUT_H
