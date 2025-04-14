#ifndef RENDER_PIPELINE_H
#define RENDER_PIPELINE_H

#include "types.h"

struct pipeline_builder pipeline_builder_new(const struct renderer *renderer);

void pipeline_builder_set_shaders(struct pipeline_builder *builder,
                                  const char *vertex_shader_path,
                                  const char *fragment_shader_path);
void pipeline_builder_add_input_binding(struct pipeline_builder *builder,
                                        u32 binding,
                                        u64 stride,
                                        VkVertexInputRate input_rate);
void pipeline_builder_add_input_attribute(struct pipeline_builder *builder,
                                          u32 binding,
                                          u32 location,
                                          VkFormat format,
                                          u32 offset);
void pipeline_builder_set_ubo_size(struct pipeline_builder *builder, u64 ubo_size);
void pipeline_builder_set_topology(struct pipeline_builder *builder, VkPrimitiveTopology topology);
void pipeline_builder_set_cull_mode(struct pipeline_builder *builder, VkCullModeFlags cull_mode);
void pipeline_builder_set_alpha_blending(struct pipeline_builder *builder, b8 value);
void pipeline_builder_add_push_constant(struct pipeline_builder *builder,
                                        VkShaderStageFlagBits shader_stage,
                                        u32 size);

b8 pipeline_builder_build(struct pipeline_builder *builder,
                          VkRenderPass render_pass,
                          struct pipeline *pipeline);

void pipeline_bind(const struct renderer *renderer, const struct pipeline *pipeline);

void pipeline_destroy(const struct device *device, struct pipeline *pipeline);

#endif // RENDER_PIPELINE_H
