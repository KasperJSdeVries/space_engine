#include "graphics_pipeline.h"
#include "assets/uniform_buffer.h"
#include "assets/vertex.h"
#include "containers/darray.h"
#include "core/defines.h"
#include "core/logging.h"
#include "renderer/pipeline_layout.h"
#include "renderer/render_pass.h"
#include "renderer/shader_module.h"
#include "renderer/vulkan.h"
#include "vulkan/vulkan_core.h"

GraphicsPipeline graphics_pipeline_new(const Swapchain *swapchain,
                                       const DepthBuffer *depth_buffer,
                                       darray(UniformBuffer) uniform_buffers,
                                       const Scene *scene,
                                       b8 is_wireframe) {
    GraphicsPipeline self = {
        .swapchain = swapchain,
        .is_wireframe = is_wireframe,
    };

    const Device *device = swapchain->device;
    VkVertexInputBindingDescription binding_description =
        vertex_binding_description();
    u32 attribute_description_count;
    vertex_attribute_descriptions(&attribute_description_count, NULL);
    VkVertexInputAttributeDescription
        attribute_descriptions[attribute_description_count];
    vertex_attribute_descriptions(&attribute_description_count,
                                  attribute_descriptions);

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &binding_description,
        .vertexAttributeDescriptionCount = attribute_description_count,
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
        .width = (f32)swapchain->extent.width,
        .height = (f32)swapchain->extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = swapchain->extent,
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
        .polygonMode =
            is_wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };

    VkPipelineDepthStencilStateCreateInfo depth_stencil = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
    };

    VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
    };

    VkPipelineColorBlendStateCreateInfo color_blending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment,
    };

    VkDescriptorPoolSize pool_sizes[] = {
        (VkDescriptorPoolSize){
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1 * darray_length(uniform_buffers),
        },
        (VkDescriptorPoolSize){
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            1 * darray_length(uniform_buffers),
        },
        (VkDescriptorPoolSize){
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            darray_length(scene->texture_samplers) *
                darray_length(uniform_buffers),
        },
    };

    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .poolSizeCount = ARRAY_SIZE(pool_sizes),
        .pPoolSizes = pool_sizes,
        .maxSets = darray_length(uniform_buffers),
    };

    vulkan_check(vkCreateDescriptorPool(device->handle,
                                        &pool_info,
                                        NULL,
                                        &self.descriptor_pool),
                 "create descriptor pool");

    VkDescriptorSetLayoutBinding layout_bindings[] = {
        (VkDescriptorSetLayoutBinding){
            .binding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        },
        (VkDescriptorSetLayoutBinding){
            .binding = 1,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .stageFlags =
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        },
        (VkDescriptorSetLayoutBinding){
            .binding = 2,
            .descriptorCount = darray_length(scene->texture_samplers),
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        },
    };

    VkDescriptorSetLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = ARRAY_SIZE(layout_bindings),
        .pBindings = layout_bindings,
    };

    vulkan_check(vkCreateDescriptorSetLayout(device->handle,
                                             &layout_info,
                                             NULL,
                                             &self.descriptor_layout),
                 "create descriptor set layout");

    VkDescriptorSetLayout layouts[darray_length(uniform_buffers)];
    for (u32 i = 0; i < darray_length(uniform_buffers); i++) {
        layouts[i] = self.descriptor_layout;
    }

    VkDescriptorSetAllocateInfo layout_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = self.descriptor_pool,
        .descriptorSetCount = darray_length(uniform_buffers),
        .pSetLayouts = layouts,
    };

    self.descriptor_sets =
        calloc(darray_length(uniform_buffers), sizeof(VkDescriptorSet));

    vulkan_check(vkAllocateDescriptorSets(device->handle,
                                          &layout_alloc_info,
                                          self.descriptor_sets),
                 "allocate descriptor sets");

    for (u32 i = 0; i < darray_length(swapchain->images); i++) {
        VkDescriptorBufferInfo uniform_buffer_info = {
            .buffer = uniform_buffers[i].buffer.handle,
            .range = VK_WHOLE_SIZE,
        };

        VkDescriptorBufferInfo material_buffer_info = {
            .buffer = scene->material_buffer.handle,
            .range = VK_WHOLE_SIZE,
        };

        VkDescriptorImageInfo
            image_infos[darray_length(scene->texture_samplers)];

        for (u32 t = 0; t < darray_length(scene->texture_samplers); t++) {
            image_infos[t] = (VkDescriptorImageInfo){
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .imageView = scene->texture_image_views[t],
                .sampler = scene->texture_samplers[t],
            };
        }

        VkWriteDescriptorSet descriptor_writes[] = {
            (VkWriteDescriptorSet){
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = self.descriptor_sets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .pBufferInfo = &uniform_buffer_info,
            },
            (VkWriteDescriptorSet){
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = self.descriptor_sets[i],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .pBufferInfo = &material_buffer_info,
            },
            (VkWriteDescriptorSet){
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = self.descriptor_sets[i],
                .dstBinding = 2,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = darray_length(scene->texture_samplers),
                .pImageInfo = image_infos,
            },
        };

        vkUpdateDescriptorSets(device->handle,
                               ARRAY_SIZE(descriptor_writes),
                               descriptor_writes,
                               0,
                               NULL);
    }

    self.pipeline_layout = pipeline_layout_new(device, self.descriptor_layout);
    self.render_pass = malloc(sizeof(RenderPass));
    *self.render_pass = render_pass_new(swapchain,
                                        depth_buffer,
                                        VK_ATTACHMENT_LOAD_OP_CLEAR,
                                        VK_ATTACHMENT_LOAD_OP_CLEAR);

    ShaderModule vert_shader =
        shader_module_new(device, "assets/shaders/Graphics.vert.spv");
    ShaderModule frag_shader =
        shader_module_new(device, "assets/shaders/Graphics.frag.spv");

    VkPipelineShaderStageCreateInfo shader_stages[] = {
        shader_module_create_shader_stage(&vert_shader,
                                          VK_SHADER_STAGE_VERTEX_BIT),
        shader_module_create_shader_stage(&frag_shader,
                                          VK_SHADER_STAGE_FRAGMENT_BIT),
    };

    VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shader_stages,
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depth_stencil,
        .pColorBlendState = &color_blending,
        .layout = self.pipeline_layout.handle,
        .renderPass = self.render_pass->handle,
        .subpass = 0,
    };

    vulkan_check(vkCreateGraphicsPipelines(device->handle,
                                           NULL,
                                           1,
                                           &pipeline_info,
                                           NULL,
                                           &self.handle),
                 "create graphics pipeline");

    return self;
}

void graphics_pipeline_destroy(GraphicsPipeline *self) {
    if (self->handle != NULL) {
        vkDestroyPipeline(self->swapchain->device->handle, self->handle, NULL);
        self->handle = NULL;
    }

    render_pass_destroy(self->render_pass);
    free(self->render_pass);
    pipeline_layout_destroy(&self->pipeline_layout);
}

VkDescriptorSet graphics_pipeline_descriptor_set(const GraphicsPipeline *self,
                                                 u64 index) {
    UNUSED(self);
    UNUSED(index);

    TODO("graphics_pipeline_descriptor_set");

    return NULL;
}
