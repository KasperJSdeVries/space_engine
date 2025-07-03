#ifndef PIPELINE_LAYOUT_H
#define PIPELINE_LAYOUT_H

#include "renderer/device.h"

typedef struct {
    VkPipelineLayout handle;
    const Device *device;
} PipelineLayout;

PipelineLayout pipeline_layout_new(const Device *device,
                                   VkDescriptorSetLayout descriptor_set_layout);
void pipeline_layout_destroy(PipelineLayout *self);

#endif // PIPELINE_LAYOUT_H
