#include "ray_tracing_pipeline.h"
#include "core/defines.h"
#include "renderer/shader_module.h"
#include "vulkan/vulkan_core.h"

RayTracingPipeline ray_tracing_pipeline_new(
    const Swapchain *swapchain,
    const TopLevelAccelerationStructure *acceleration_structure,
    const ImageView *accumulation_image_view,
    const ImageView *output_image_view,
    darray(UniformBuffer) uniform_buffers,
    const Scene *scene) {
    RayTracingPipeline self = {
        .swapchain = swapchain,
    };

    const Device *device = swapchain->device;

    VkDescriptorPoolSize pool_sizes[] = {
        (VkDescriptorPoolSize){
            .type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
            .descriptorCount = 1 * darray_length(uniform_buffers),
        },
        (VkDescriptorPoolSize){
            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = 1 * darray_length(uniform_buffers),
        },
        (VkDescriptorPoolSize){
            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = 1 * darray_length(uniform_buffers),
        },
        (VkDescriptorPoolSize){
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1 * darray_length(uniform_buffers),
        },
        (VkDescriptorPoolSize){
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1 * darray_length(uniform_buffers),
        },
        (VkDescriptorPoolSize){
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1 * darray_length(uniform_buffers),
        },
        (VkDescriptorPoolSize){
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1 * darray_length(uniform_buffers),
        },
        (VkDescriptorPoolSize){
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1 * darray_length(uniform_buffers),
        },
        (VkDescriptorPoolSize){
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = darray_length(scene->texture_samplers) *
                               darray_length(uniform_buffers),
        },
        (VkDescriptorPoolSize){
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1 * darray_length(uniform_buffers),
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
            .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
            .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
        },
        (VkDescriptorSetLayoutBinding){
            .binding = 1,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
        },
        (VkDescriptorSetLayoutBinding){
            .binding = 2,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
        },
        (VkDescriptorSetLayoutBinding){
            .binding = 3,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .stageFlags =
                VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR,
        },
        (VkDescriptorSetLayoutBinding){
            .binding = 4,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
        },
        (VkDescriptorSetLayoutBinding){
            .binding = 5,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
        },
        (VkDescriptorSetLayoutBinding){
            .binding = 6,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
        },
        (VkDescriptorSetLayoutBinding){
            .binding = 7,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
        },
        (VkDescriptorSetLayoutBinding){
            .binding = 8,
            .descriptorCount = darray_length(scene->texture_samplers),
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
        },
        (VkDescriptorSetLayoutBinding){
            .binding = 9,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
                          VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
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
        const VkAccelerationStructureKHR acceleration_structure_handle =
            acceleration_structure->handle;
        VkWriteDescriptorSetAccelerationStructureKHR structure_info = {
            .sType =
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
            .accelerationStructureCount = 1,
            .pAccelerationStructures = &acceleration_structure_handle,
        };

        VkDescriptorImageInfo accumulation_image_info = {
            .imageView = accumulation_image_view->handle,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        };

        VkDescriptorImageInfo output_image_info = {
            .imageView = output_image_view->handle,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        };

        VkDescriptorBufferInfo uniform_buffer_info = {
            .buffer = uniform_buffers[i].buffer.handle,
            .range = VK_WHOLE_SIZE,
        };

        VkDescriptorBufferInfo vertex_buffer_info = {
            .buffer = scene->vertex_buffer.handle,
            .range = VK_WHOLE_SIZE,
        };

        VkDescriptorBufferInfo index_buffer_info = {
            .buffer = scene->index_buffer.handle,
            .range = VK_WHOLE_SIZE,
        };

        VkDescriptorBufferInfo material_buffer_info = {
            .buffer = scene->material_buffer.handle,
            .range = VK_WHOLE_SIZE,
        };

        VkDescriptorBufferInfo offset_buffer_info = {
            .buffer = scene->offset_buffer.handle,
            .range = VK_WHOLE_SIZE,
        };

        VkDescriptorImageInfo
            image_infos[darray_length(scene->texture_samplers)];
        for (u32 i = 0; i < darray_length(scene->texture_samplers); i++) {
            image_infos[i] = (VkDescriptorImageInfo){
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .imageView = scene->texture_image_views[i],
                .sampler = scene->texture_samplers[i],
            };
        }

        VkWriteDescriptorSet descriptor_writes[] = {
            (VkWriteDescriptorSet){
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = self.descriptor_sets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
                .descriptorCount = 1,
                .pNext = &structure_info,
            },
            (VkWriteDescriptorSet){
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = self.descriptor_sets[i],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                .descriptorCount = 1,
                .pImageInfo = &accumulation_image_info,
            },
            (VkWriteDescriptorSet){
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = self.descriptor_sets[i],
                .dstBinding = 2,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                .descriptorCount = 1,
                .pImageInfo = &output_image_info,
            },
            (VkWriteDescriptorSet){
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = self.descriptor_sets[i],
                .dstBinding = 3,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .pBufferInfo = &uniform_buffer_info,
            },
            (VkWriteDescriptorSet){
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = self.descriptor_sets[i],
                .dstBinding = 4,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .pBufferInfo = &vertex_buffer_info,
            },
            (VkWriteDescriptorSet){
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = self.descriptor_sets[i],
                .dstBinding = 5,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .pBufferInfo = &index_buffer_info,
            },
            (VkWriteDescriptorSet){
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = self.descriptor_sets[i],
                .dstBinding = 6,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .pBufferInfo = &material_buffer_info,
            },
            (VkWriteDescriptorSet){
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = self.descriptor_sets[i],
                .dstBinding = 7,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .pBufferInfo = &offset_buffer_info,
            },
            (VkWriteDescriptorSet){
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = self.descriptor_sets[i],
                .dstBinding = 8,
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

    self.layout = pipeline_layout_new(device, self.descriptor_layout);

    ShaderModule ray_gen_shader =
        shader_module_new(device, "assets/shaders/RayTracing.rgen.spv");
    ShaderModule miss_shader =
        shader_module_new(device, "assets/shaders/RayTracing.rmiss.spv");
    ShaderModule closest_hit_shader =
        shader_module_new(device, "assets/shaders/RayTracing.rchit.spv");

    VkPipelineShaderStageCreateInfo shader_stages[] = {
        shader_module_create_shader_stage(&ray_gen_shader,
                                          VK_SHADER_STAGE_RAYGEN_BIT_KHR),
        shader_module_create_shader_stage(&miss_shader,
                                          VK_SHADER_STAGE_MISS_BIT_KHR),
        shader_module_create_shader_stage(&closest_hit_shader,
                                          VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR),
    };

    VkRayTracingShaderGroupCreateInfoKHR ray_gen_group_info = {
        .sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
        .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
        .generalShader = 0,
        .closestHitShader = VK_SHADER_UNUSED_KHR,
        .anyHitShader = VK_SHADER_UNUSED_KHR,
        .intersectionShader = VK_SHADER_UNUSED_KHR,
    };
    self.ray_gen_index = 0;

    VkRayTracingShaderGroupCreateInfoKHR miss_group_info = {
        .sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
        .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
        .generalShader = 1,
        .closestHitShader = VK_SHADER_UNUSED_KHR,
        .anyHitShader = VK_SHADER_UNUSED_KHR,
        .intersectionShader = VK_SHADER_UNUSED_KHR,
    };
    self.miss_index = 1;

    VkRayTracingShaderGroupCreateInfoKHR triangle_hit_group_info = {
        .sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
        .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR,
        .generalShader = VK_SHADER_UNUSED_KHR,
        .closestHitShader = 2,
        .anyHitShader = VK_SHADER_UNUSED_KHR,
        .intersectionShader = VK_SHADER_UNUSED_KHR,
    };
    self.triangle_hit_group_index = 2;

    VkRayTracingShaderGroupCreateInfoKHR groups[] = {
        ray_gen_group_info,
        miss_group_info,
        triangle_hit_group_info,
    };

    VkRayTracingPipelineCreateInfoKHR pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
        .stageCount = ARRAY_SIZE(shader_stages),
        .pStages = shader_stages,
        .groupCount = ARRAY_SIZE(groups),
        .pGroups = groups,
        .maxPipelineRayRecursionDepth = 1,
        .layout = self.layout.handle,
    };

    vulkan_check(
        device->procedures.vkCreateRayTracingPipelinesKHR(device->handle,
                                                          NULL,
                                                          NULL,
                                                          1,
                                                          &pipeline_info,
                                                          NULL,
                                                          &self.handle),
        "create ray tracing pipeline");

    return self;
}

void ray_tracing_pipeline_destroy(RayTracingPipeline *self) {
    if (self->handle != NULL) {
        vkDestroyPipeline(self->swapchain->device->handle, self->handle, NULL);
        self->handle = NULL;
    }

    pipeline_layout_destroy(&self->layout);
}
