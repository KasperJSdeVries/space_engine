#ifndef RAY_TRACING_PIPELINE_H
#define RAY_TRACING_PIPELINE_H

#include "assets/scene.h"
#include "assets/uniform_buffer.h"
#include "renderer/pipeline_layout.h"
#include "renderer/swapchain.h"
#include "renderer/top_level_acceleration_structure.h"

typedef struct {
    const Swapchain *swapchain;
    VkPipeline handle;
    PipelineLayout layout;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSetLayout descriptor_layout;
    VkDescriptorSet *descriptor_sets;
    u32 ray_gen_index;
    u32 miss_index;
    u32 triangle_hit_group_index;
    u32 procedural_hit_group_index;
} RayTracingPipeline;

RayTracingPipeline ray_tracing_pipeline_new(
    const Swapchain *swapchain,
    const TopLevelAccelerationStructure *acceleration_structure,
    const ImageView *accumulation_image_view,
    const ImageView *output_image_view,
    darray(UniformBuffer) uniform_buffers,
    const Scene *scene);
void ray_tracing_pipeline_destroy(RayTracingPipeline *self);

#endif // RAY_TRACING_PIPELINE_H
