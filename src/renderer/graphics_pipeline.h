#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H

#include "assets/scene.h"
#include "assets/uniform_buffer.h"
#include "renderer/pipeline_layout.h"
#include "renderer/render_pass.h"
#include "renderer/swapchain.h"

typedef struct {
    VkPipeline handle;
    const Swapchain *swapchain;
    PipelineLayout pipeline_layout;
    RenderPass *render_pass;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSetLayout descriptor_layout;
    VkDescriptorSet *descriptor_sets;
    b8 is_wireframe;
} GraphicsPipeline;

GraphicsPipeline graphics_pipeline_new(const Swapchain *swapchain,
                                       const DepthBuffer *depth_buffer,
                                       darray(UniformBuffer) uniform_buffers,
                                       const Scene *scene,
                                       b8 is_wireframe);
void graphics_pipeline_destroy(GraphicsPipeline *self);

VkDescriptorSet graphics_pipeline_descriptor_set(const GraphicsPipeline *self,
                                                 u64 index);

#endif // GRAPHICS_PIPELINE_H
