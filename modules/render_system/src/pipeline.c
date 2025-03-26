#include "pipeline.h"

#include "math/vec3.h"
#include "types.h"

#include "core/assert.h"
#include "core/defines.h"

#include "vulkan/vulkan_core.h"

#include <stdio.h>

b8 shader_module_create(const struct device *device,
                        const char *file_path,
                        VkShaderModule *shader_module);

b8 graphics_pipeline_create(const struct renderer *renderer,
                            struct graphics_pipeline_config config,
                            struct pipeline *pipeline) {
    VkShaderModule vert_shader_module;
    VkShaderModule frag_shader_module;

    if (!shader_module_create(&renderer->device, config.vertex_shader_path, &vert_shader_module)) {
        return false;
    }
    if (!shader_module_create(&renderer->device,
                              config.fragment_shader_path,
                              &frag_shader_module)) {
        return false;
    }

    VkPipelineShaderStageCreateInfo shader_stages[] = {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vert_shader_module,
            .pName = "main",
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = frag_shader_module,
            .pName = "main",
        },
    };

    VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamic_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = ARRAY_SIZE(dynamic_states),
        .pDynamicStates = dynamic_states,
    };

    VkVertexInputBindingDescription binding_descriptions[] = {
        {
            .binding = 0,
            .stride = config.vertex_size,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        },
    };

    VkVertexInputAttributeDescription attribute_descriptions[] = {
        {
            .binding = 0,
            .location = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = 0,
        },
    };

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = ARRAY_SIZE(binding_descriptions),
        .pVertexBindingDescriptions = binding_descriptions,
        .vertexAttributeDescriptionCount = ARRAY_SIZE(attribute_descriptions),
        .pVertexAttributeDescriptions = attribute_descriptions,
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (f32)renderer->swapchain.extent.width,
        .height = (f32)renderer->swapchain.extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = renderer->swapchain.extent,
    };

    VkPipelineViewportStateCreateInfo viewport_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .minSampleShading = 1.0f,
    };

    VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
    };

    VkPipelineColorBlendStateCreateInfo color_blending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment,
    };

    VkDescriptorSetLayoutBinding uboLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    };

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &uboLayoutBinding,
    };

    if (!ASSERT(vkCreateDescriptorSetLayout(renderer->device.handle,
                                            &descriptor_set_layout_info,
                                            NULL,
                                            &pipeline->descriptor_set_layout) == VK_SUCCESS)) {
        return false;
    }

    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &pipeline->descriptor_set_layout,
    };

    if (!ASSERT(vkCreatePipelineLayout(renderer->device.handle,
                                       &pipeline_layout_info,
                                       NULL,
                                       &pipeline->layout) == VK_SUCCESS)) {
        return false;
    }

    VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = ARRAY_SIZE(shader_stages),
        .pStages = shader_stages,
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = NULL,
        .pColorBlendState = &color_blending,
        .pDynamicState = &dynamic_state,
        .layout = pipeline->layout,
        .renderPass = renderer->renderpass.handle,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };

    if (!ASSERT(vkCreateGraphicsPipelines(renderer->device.handle,
                                          NULL,
                                          1,
                                          &pipeline_info,
                                          NULL,
                                          &pipeline->handle) == VK_SUCCESS)) {
        return false;
    }

    vkDestroyShaderModule(renderer->device.handle, vert_shader_module, NULL);
    vkDestroyShaderModule(renderer->device.handle, frag_shader_module, NULL);

    VkDescriptorPoolSize pool_size = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = MAX_FRAMES_IN_FLIGHT,
    };

    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 1,
        .pPoolSizes = &pool_size,
        .maxSets = MAX_FRAMES_IN_FLIGHT,
    };

    if (!ASSERT(vkCreateDescriptorPool(renderer->device.handle,
                                       &pool_info,
                                       NULL,
                                       &pipeline->descriptor_pool) == VK_SUCCESS)) {
        return false;
    }

    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        layouts[i] = pipeline->descriptor_set_layout;
    }

    VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = pipeline->descriptor_pool,
        .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
        .pSetLayouts = layouts,
    };

    if (!ASSERT(vkAllocateDescriptorSets(renderer->device.handle,
                                         &descriptor_set_allocate_info,
                                         pipeline->descriptor_sets) == VK_SUCCESS)) {
        return false;
    }

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo buffer_info = {
            .buffer = renderer->ubo_buffers[i].handle,
            .offset = 0,
            .range = sizeof(struct uniform_buffer_object),
        };

        VkWriteDescriptorSet descriptor_write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = pipeline->descriptor_sets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .pBufferInfo = &buffer_info,
        };

        vkUpdateDescriptorSets(renderer->device.handle, 1, &descriptor_write, 0, NULL);
    }

    return true;
}

void pipeline_destroy(const struct device *device, struct pipeline *pipeline) {
    vkDestroyPipeline(device->handle, pipeline->handle, NULL);
    pipeline->handle = VK_NULL_HANDLE;
    vkDestroyPipelineLayout(device->handle, pipeline->layout, NULL);
    pipeline->layout = VK_NULL_HANDLE;
    vkDestroyDescriptorPool(device->handle, pipeline->descriptor_pool, NULL);
    pipeline->descriptor_pool = VK_NULL_HANDLE;
    vkDestroyDescriptorSetLayout(device->handle, pipeline->descriptor_set_layout, NULL);
    pipeline->descriptor_set_layout = VK_NULL_HANDLE;
}

void pipeline_bind(const struct renderer *renderer, const struct pipeline *pipeline) {
    vkCmdBindDescriptorSets(renderer->commandbuffers[renderer->current_frame].handle,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline->layout,
                            0,
                            1,
                            &pipeline->descriptor_sets[renderer->current_frame],
                            0,
                            NULL);

    vkCmdBindPipeline(renderer->commandbuffers[renderer->current_frame].handle,
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline->handle);
}

b8 shader_module_create(const struct device *device,
                        const char *file_path,
                        VkShaderModule *shader_module) {
    FILE *fp = fopen(file_path, "rb");
    if (!ASSERT(fp != NULL)) {
        return false;
    }

    fseek(fp, 0, SEEK_END);
    u64 size = ftell(fp);
    rewind(fp);

    char buffer[size];
    fread(buffer, sizeof(char), size, fp);

    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode = (u32 *)buffer,
    };

    if (!ASSERT(vkCreateShaderModule(device->handle, &create_info, NULL, shader_module) ==
                VK_SUCCESS)) {
        return false;
    }

    return true;
}
