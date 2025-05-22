#include "pipeline.h"

#include "buffer.h"
#include "core/logging.h"
#include "types.h"

#include "core/darray.h"
#include "core/defines.h"

#include <stdio.h>
#include <string.h>

b8 shader_module_create(const struct device *device,
                        const char *file_path,
                        VkShaderModule *shader_module);

pipeline_builder pipeline_builder_new(const struct renderer *renderer) {
    return (pipeline_builder){
        .renderer = renderer,
        .vertex_input_bindings = darray_new(VkVertexInputBindingDescription),
        .vertex_input_attributes =
            darray_new(VkVertexInputAttributeDescription),
        .cull_mode = VK_CULL_MODE_BACK_BIT,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .push_constant_ranges = darray_new(VkPushConstantRange),
    };
}

void pipeline_builder_set_shaders(pipeline_builder *builder,
                                  const char *vertex_shader_path,
                                  const char *fragment_shader_path) {
    if (!shader_module_create(&builder->renderer->device,
                              vertex_shader_path,
                              &builder->vertex_shader_module)) {
        return;
    }

    VkPipelineShaderStageCreateInfo vertex_shader_stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = builder->vertex_shader_module,
        .pName = "main",
    };

    builder->shader_stages[0] = vertex_shader_stage_info;

    if (!shader_module_create(&builder->renderer->device,
                              fragment_shader_path,
                              &builder->fragment_shader_module)) {
        return;
    }

    VkPipelineShaderStageCreateInfo fragment_shader_stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = builder->fragment_shader_module,
        .pName = "main",
    };

    builder->shader_stages[1] = fragment_shader_stage_info;
}

void pipeline_builder_add_input_binding(pipeline_builder *builder,
                                        u32 binding,
                                        u64 stride,
                                        VkVertexInputRate input_rate) {
    VkVertexInputBindingDescription description = {
        .binding = binding,
        .stride = stride,
        .inputRate = input_rate,
    };

    darray_push(builder->vertex_input_bindings, description);
}

void pipeline_builder_add_input_attribute(pipeline_builder *builder,
                                          u32 binding,
                                          u32 location,
                                          VkFormat format,
                                          u32 offset) {
    VkVertexInputAttributeDescription description = {
        .binding = binding,
        .location = location,
        .format = format,
        .offset = offset,
    };

    darray_push(builder->vertex_input_attributes, description);
}

void pipeline_builder_set_ubo_size(pipeline_builder *builder, u64 ubo_size) {
    builder->ubo_size = ubo_size;
}

void pipeline_builder_set_topology(pipeline_builder *builder,
                                   VkPrimitiveTopology topology) {
    builder->topology = topology;
}

void pipeline_builder_set_cull_mode(pipeline_builder *builder,
                                    VkCullModeFlags cull_mode) {
    builder->cull_mode = cull_mode;
}

void pipeline_builder_set_alpha_blending(pipeline_builder *builder, b8 value) {
    builder->enable_alpha_blending = value;
}

void pipeline_builder_add_push_constant(pipeline_builder *builder,
                                        VkShaderStageFlagBits shader_stage,
                                        u32 size) {
    VkPushConstantRange range = {
        .stageFlags = shader_stage,
        .offset = 0,
        .size = size,
    };

    darray_push(builder->push_constant_ranges, range);
}

b8 pipeline_builder_build(pipeline_builder *builder,
                          VkRenderPass render_pass,
                          struct pipeline *pipeline) {
    memset(pipeline, 0, sizeof(struct pipeline));

    if (builder->ubo_size) {
        VkDescriptorSetLayoutBinding layout_bindings[] = {
            {
                .binding = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pImmutableSamplers = NULL,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            },
        };

        VkDescriptorSetLayoutCreateInfo layout_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount =
                sizeof(VkDescriptorSetLayoutBinding) / sizeof(layout_bindings),
            .pBindings = layout_bindings,
        };

        if (vkCreateDescriptorSetLayout(builder->renderer->device.handle,
                                        &layout_info,
                                        NULL,
                                        &pipeline->descriptor_set_layout) !=
            VK_SUCCESS) {
            return false;
        }
    }

    VkPipelineVertexInputStateCreateInfo vertex_input_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount =
            darray_length(builder->vertex_input_bindings),
        .pVertexBindingDescriptions = builder->vertex_input_bindings,
        .vertexAttributeDescriptionCount =
            darray_length(builder->vertex_input_attributes),
        .pVertexAttributeDescriptions = builder->vertex_input_attributes,
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = builder->topology,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    uint32_t dynamic_state_count =
        sizeof(dynamic_states) / sizeof(VkDynamicState);

    VkPipelineDynamicStateCreateInfo dynamic_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = dynamic_state_count,
        .pDynamicStates = dynamic_states,
    };

    VkPipelineViewportStateCreateInfo viewport_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    VkPipelineRasterizationStateCreateInfo rasterization_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = builder->cull_mode,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
    };

    VkPipelineMultisampleStateCreateInfo multisample_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .minSampleShading = 1.0f,
    };

    VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = builder->enable_alpha_blending ? VK_TRUE : VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
    };

    VkPipelineColorBlendStateCreateInfo color_blend_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment,
    };

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
    };

    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pushConstantRangeCount = darray_length(builder->push_constant_ranges),
        .pPushConstantRanges = builder->push_constant_ranges,
    };

    if (builder->ubo_size) {
        pipeline_layout_create_info.setLayoutCount = 1;
        pipeline_layout_create_info.pSetLayouts =
            &pipeline->descriptor_set_layout;
    }

    if (vkCreatePipelineLayout(builder->renderer->device.handle,
                               &pipeline_layout_create_info,
                               NULL,
                               &pipeline->layout) != VK_SUCCESS) {
        return false;
    }

    VkGraphicsPipelineCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = builder->shader_stages,
        .pVertexInputState = &vertex_input_state,
        .pInputAssemblyState = &input_assembly_state,
        .pDynamicState = &dynamic_state,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterization_state,
        .pMultisampleState = &multisample_state,
        .pColorBlendState = &color_blend_state,
        .pDepthStencilState = &depth_stencil_state,
        .layout = pipeline->layout,
        .renderPass = render_pass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };

    if (vkCreateGraphicsPipelines(builder->renderer->device.handle,
                                  VK_NULL_HANDLE,
                                  1,
                                  &create_info,
                                  NULL,
                                  &pipeline->handle) != VK_SUCCESS) {
        return false;
    }

    if (builder->ubo_size != 0) {
        render_buffer_create(&builder->renderer->device,
                             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             builder->ubo_size,
                             &pipeline->uniform_buffer);

        render_buffer_map_memory(&builder->renderer->device,
                                 &pipeline->uniform_buffer,
                                 &pipeline->uniform_buffer_mapped);

        VkDescriptorPoolSize pool_sizes[] = {
            {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = MAX_FRAMES_IN_FLIGHT,
            },
        };

        VkDescriptorPoolCreateInfo pool_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .poolSizeCount = ARRAY_SIZE(pool_sizes),
            .pPoolSizes = pool_sizes,
            .maxSets = MAX_FRAMES_IN_FLIGHT,
        };

        if (vkCreateDescriptorPool(builder->renderer->device.handle,
                                   &pool_info,
                                   NULL,
                                   &pipeline->descriptor_pool) != VK_SUCCESS) {
            return false;
        }

        VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            layouts[i] = pipeline->descriptor_set_layout;
        }

        VkDescriptorSetAllocateInfo alloc_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = pipeline->descriptor_pool,
            .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
            .pSetLayouts = layouts,
        };

        if (vkAllocateDescriptorSets(builder->renderer->device.handle,
                                     &alloc_info,
                                     pipeline->descriptor_sets) != VK_SUCCESS) {
            return false;
        }

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo buffer_info = {
                .buffer = pipeline->uniform_buffer.handle,
                .offset = 0,
                .range = builder->ubo_size,
            };

            VkWriteDescriptorSet descriptor_writes[] = {
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = pipeline->descriptor_sets[i],
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1,
                    .pBufferInfo = &buffer_info,
                },
            };

            vkUpdateDescriptorSets(builder->renderer->device.handle,
                                   sizeof(descriptor_writes) /
                                       sizeof(VkWriteDescriptorSet),
                                   descriptor_writes,
                                   0,
                                   NULL);
        }
    }

    vkDestroyShaderModule(builder->renderer->device.handle,
                          builder->vertex_shader_module,
                          NULL);
    vkDestroyShaderModule(builder->renderer->device.handle,
                          builder->fragment_shader_module,
                          NULL);

    return pipeline;
}

void pipeline_bind(const struct renderer *renderer,
                   const struct pipeline *pipeline) {
    if (pipeline->uniform_buffer.size > 0) {
        vkCmdBindDescriptorSets(
            renderer->commandbuffers[renderer->current_frame].handle,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline->layout,
            0,
            1,
            &pipeline->descriptor_sets[renderer->current_frame],
            0,
            NULL);
    } else {
        TODO("No Uniform Buffer bound to pipeline");
    }
    vkCmdBindPipeline(renderer->commandbuffers[renderer->current_frame].handle,
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline->handle);
}

void pipeline_destroy(const struct device *device, struct pipeline *pipeline) {
    if (pipeline->uniform_buffer.size > 0) {
        render_buffer_destroy(device, &pipeline->uniform_buffer);

        vkDestroyDescriptorPool(device->handle,
                                pipeline->descriptor_pool,
                                NULL);
        vkDestroyDescriptorSetLayout(device->handle,
                                     pipeline->descriptor_set_layout,
                                     NULL);
    }

    vkDestroyPipeline(device->handle, pipeline->handle, NULL);
    vkDestroyPipelineLayout(device->handle, pipeline->layout, NULL);
}

b8 shader_module_create(const struct device *device,
                        const char *file_path,
                        VkShaderModule *shader_module) {
    FILE *fp = fopen(file_path, "rb");
    if (fp == NULL) {
        return false;
    }

    fseek(fp, 0, SEEK_END);
    u64 size = ftell(fp);
    rewind(fp);

    char buffer[size];
    if (fread(buffer, sizeof(char), size, fp) != size) {
        return false;
    }

    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode = (u32 *)buffer,
    };

    if (vkCreateShaderModule(device->handle,
                             &create_info,
                             NULL,
                             shader_module) != VK_SUCCESS) {
        return false;
    }

    return true;
}
