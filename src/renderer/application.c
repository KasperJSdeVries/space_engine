#include "application.h"

#include "assets/scene.h"
#include "assets/uniform_buffer.h"
#include "cglm/util.h"
#include "containers/darray.h"
#include "core/defines.h"
#include "core/logging.h"
#include "renderer/buffer.h"
#include "renderer/command_buffers.h"
#include "renderer/command_pool.h"
#include "renderer/depth_buffer.h"
#include "renderer/device.h"
#include "renderer/device_memory.h"
#include "renderer/fence.h"
#include "renderer/framebuffer.h"
#include "renderer/graphics_pipeline.h"
#include "renderer/image.h"
#include "renderer/instance.h"
#include "renderer/ray_tracing_pipeline.h"
#include "renderer/semaphore.h"
#include "renderer/shader_binding_table.h"
#include "renderer/single_time_commands.h"
#include "renderer/swapchain.h"
#include "renderer/top_level_acceleration_structure.h"
#include "renderer/window.h"
#include "vulkan/vulkan_core.h"

#include <stdlib.h>

static void draw_frame(void *);
static VkPhysicalDevice choose_physical_device(Application *application);
static void create_swapchain(Application *self);
static void delete_swapchain(Application *self);
static void recreate_swapchain(Application *self);

Application application_new(WindowConfig window_config,
                            Scene *scene,
                            VkPresentModeKHR present_mode,
                            b8 enable_validation_layers) {
    Application self = {
        .present_mode = present_mode,
        .scene = scene,
        .number_of_samples = 8,
        .total_number_of_samples = 0,
    };

    darray(const char *) validation_layers = darray_new(const char *);
    if (enable_validation_layers) {
        darray_push(validation_layers, "VK_LAYER_KHRONOS_validation");
    }

    self.window = window_new(window_config);
    self.instance =
        instance_new(self.window, validation_layers, VK_API_VERSION_1_3);
    self.debug_utils_messenger =
        enable_validation_layers
            ? debug_utils_messenger_new(
                  &self.instance,
                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
            : NULL;
    self.surface = surface_new(&self.instance);

    VkPhysicalDevice physical_device = choose_physical_device(&self);

    if (self.device != NULL) {
        LOG_FATAL("physical device has already been set");
        exit(EXIT_FAILURE);
    }

    darray(const char *) required_extensions = darray_new(const char *);
    darray_push(required_extensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    darray_push(required_extensions,
                VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    darray_push(required_extensions,
                VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    darray_push(required_extensions,
                VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    darray_push(required_extensions, VK_KHR_SHADER_CLOCK_EXTENSION_NAME);

    VkPhysicalDeviceFeatures device_features = {
        .fillModeNonSolid = true,
        .samplerAnisotropy = true,
        .shaderInt64 = true,
    };

    VkPhysicalDeviceShaderClockFeaturesKHR shader_clock_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR,
        .pNext = NULL,
        .shaderSubgroupClock = true,
    };

    VkPhysicalDeviceBufferDeviceAddressFeatures buffer_device_address_features =
        {
            .sType =
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
            .pNext = &shader_clock_features,
            .bufferDeviceAddress = true,
        };

    VkPhysicalDeviceDescriptorIndexingFeatures indexing_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
        .pNext = &buffer_device_address_features,
        .runtimeDescriptorArray = true,
        .shaderSampledImageArrayNonUniformIndexing = true,
    };

    VkPhysicalDeviceAccelerationStructureFeaturesKHR
        acceleration_structure_features = {
            .sType =
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
            .pNext = &indexing_features,
            .accelerationStructure = true,
        };

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_features = {
        .sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
        .pNext = &acceleration_structure_features,
        .rayTracingPipeline = true,
    };

    self.device = malloc(sizeof(Device));
    *self.device = device_new(physical_device,
                              &self.surface,
                              required_extensions,
                              device_features,
                              &ray_tracing_features);

    self.command_pool =
        command_pool_new(self.device, self.device->graphics_family_index, true);

    scene_generate_buffers(self.scene, &self.command_pool);

    Buffer bottom_scratch_buffer = {0};
    DeviceMemory bottom_scratch_buffer_memory = {0};
    Buffer top_scratch_buffer = {0};
    DeviceMemory top_scratch_buffer_memory = {0};

    single_time_commands_submit(&self.command_pool) {
        LOG_TRACE("creating acceleration structures");
        // Bottom level AS
        {
            u32 vertex_offset = 0;
            u32 index_offset = 0;

            self.bottom_as = darray_new(BottomLevelAccelerationStructure);

            for (u32 i = 0; i < darray_length(self.scene->models); i++) {
                const Model *model = &self.scene->models[i];

                u32 vertex_count = darray_length(model->vertices);
                u32 index_count = darray_length(model->indices);
                BottomLevelGeometry geometries = bottom_level_geometry_new();

                bottom_level_geometry_add_triangle_geometry(&geometries,
                                                            scene,
                                                            vertex_offset,
                                                            vertex_count,
                                                            index_offset,
                                                            index_count,
                                                            true);

                darray_push(
                    self.bottom_as,
                    bottom_level_acceleration_structure_new(self.device,
                                                            geometries));

                vertex_offset += vertex_count * sizeof(Vertex);
                index_offset += index_count * sizeof(u32);
            }

            VkAccelerationStructureBuildSizesInfoKHR total = {0};
            for (u32 i = 0; i < darray_length(self.bottom_as); i++) {
                total.accelerationStructureSize +=
                    self.bottom_as[i]
                        .build_sizes_info.accelerationStructureSize;
                total.buildScratchSize +=
                    self.bottom_as[i].build_sizes_info.buildScratchSize;
                total.updateScratchSize +=
                    self.bottom_as[i].build_sizes_info.updateScratchSize;
            }

            self.bottom_buffer = buffer_new(
                self.device,
                total.accelerationStructureSize,
                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
            self.bottom_buffer_memory =
                buffer_allocate_memory(&self.bottom_buffer,
                                       VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            bottom_scratch_buffer = buffer_new(
                self.device,
                total.buildScratchSize,
                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
            bottom_scratch_buffer_memory =
                buffer_allocate_memory(&bottom_scratch_buffer,
                                       VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            VkDeviceSize result_offset = 0;
            VkDeviceSize scratch_offset = 0;

            for (u64 i = 0; i < darray_length(self.bottom_as); i++) {
                bottom_level_acceleration_structure_generate(
                    &self.bottom_as[i],
                    command_buffer,
                    &bottom_scratch_buffer,
                    scratch_offset,
                    &self.bottom_buffer,
                    result_offset);

                result_offset +=
                    self.bottom_as[i]
                        .build_sizes_info.accelerationStructureSize;
                scratch_offset +=
                    self.bottom_as[i].build_sizes_info.buildScratchSize;
            }
        }
        // Top level AS
        {
            darray(VkAccelerationStructureInstanceKHR) instances =
                darray_new(VkAccelerationStructureInstanceKHR);

            u32 instance_id = 0;

            for (u32 i = 0; i < darray_length(self.scene->models); i++) {
                darray_push(instances,
                            top_level_acceleration_structure_create_instance(
                                &self.bottom_as[instance_id],
                                mat4_identity(),
                                instance_id,
                                0));
                instance_id++;
            }

            create_device_buffer(
                &self.command_pool,
                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                instances,
                &self.instances_buffer,
                &self.instances_buffer_memory);

            acceleration_structure_memory_barrier(command_buffer);

            self.top_as = malloc(sizeof(TopLevelAccelerationStructure));
            *self.top_as = top_level_acceleration_structure_new(
                self.device,
                buffer_get_device_address(&self.instances_buffer),
                darray_length(instances));

            self.top_buffer = buffer_new(
                self.device,
                self.top_as->build_sizes_info.accelerationStructureSize,
                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
            self.top_buffer_memory =
                buffer_allocate_memory(&self.top_buffer,
                                       0,
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            top_scratch_buffer = buffer_new(
                self.device,
                self.top_as->build_sizes_info.buildScratchSize,
                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
            top_scratch_buffer_memory =
                buffer_allocate_memory(&top_scratch_buffer,
                                       VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            top_level_acceleration_structure_generate(self.top_as,
                                                      command_buffer,
                                                      &top_scratch_buffer,
                                                      0,
                                                      &self.top_buffer,
                                                      0);
        }
    }

    buffer_destroy(&top_scratch_buffer);
    device_memory_destroy(&top_scratch_buffer_memory);
    buffer_destroy(&bottom_scratch_buffer);
    device_memory_destroy(&bottom_scratch_buffer_memory);

    create_swapchain(&self);

    return self;
}

void application_destroy(Application *self) {
    delete_swapchain(self);

    top_level_acceleration_structure_destroy(self->top_as);
    free(self->top_as);

    command_pool_destroy(&self->command_pool);
    device_destroy(self->device);
    free(self->device);
    surface_destroy(&self->surface);
    debug_utils_messenger_destroy(self->debug_utils_messenger);
    instance_destroy(&self->instance);
    window_destroy(self->window);
}

void application_run(Application *self) {
    if (self->device == NULL) {
        LOG_FATAL("physical device has not been set");
        exit(EXIT_FAILURE);
    }

    self->current_frame = 0;

    window_set_context(self->window, self);
    window_set_draw_frame(self->window, draw_frame);
    window_run(self->window);
    device_wait_idle(self->device);
}

static void draw_frame(void *ctx) {
    Application *self = ctx;
    u64 no_timeout = UINT64_MAX;

    self->number_of_samples =
        CLAMP((i64)(64 * 1024 - self->total_number_of_samples), 0, 8);
    self->total_number_of_samples += self->number_of_samples;

    Fence *in_flight_fence = &self->in_flight_fences[self->current_frame];
    VkSemaphore image_available_semaphore =
        self->image_available_semaphores[self->current_frame].handle;
    VkSemaphore render_finished_semaphore =
        self->render_finished_semaphores[self->current_frame].handle;

    fence_wait(in_flight_fence, no_timeout);

    u32 image_index;
    VkResult result = vkAcquireNextImageKHR(self->device->handle,
                                            self->swapchain.handle,
                                            no_timeout,
                                            image_available_semaphore,
                                            NULL,
                                            &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreate_swapchain(self);
        return;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        LOG_FATAL("failed to acquire next image (%s)",
                  vulkan_result_to_string(result));
        exit(EXIT_FAILURE);
    }

    VkCommandBuffer command_buffer =
        command_buffer_begin(&self->command_buffers, self->current_frame);

#define RAY_TRACING 1

#if RAY_TRACING
    {
        VkExtent2D extent = self->swapchain.extent;

        VkDescriptorSet descriptor_sets[] = {
            self->ray_tracing_pipeline.descriptor_sets[self->current_frame]};

        VkImageSubresourceRange sub_resource_range = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };

        image_memory_barrier_insert(command_buffer,
                                    self->accumulation_image.handle,
                                    sub_resource_range,
                                    0,
                                    VK_ACCESS_SHADER_WRITE_BIT,
                                    VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_GENERAL);

        image_memory_barrier_insert(command_buffer,
                                    self->output_image.handle,
                                    sub_resource_range,
                                    0,
                                    VK_ACCESS_SHADER_WRITE_BIT,
                                    VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_GENERAL);

        vkCmdBindPipeline(command_buffer,
                          VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                          self->ray_tracing_pipeline.handle);
        vkCmdBindDescriptorSets(command_buffer,
                                VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                                self->ray_tracing_pipeline.layout.handle,
                                0,
                                1,
                                descriptor_sets,
                                0,
                                NULL);

        VkStridedDeviceAddressRegionKHR ray_gen_shader_binding_table = {
            .deviceAddress = shader_binding_table_ray_gen_device_address(
                &self->shader_binding_table),
            .stride = self->shader_binding_table.ray_gen_entry_size,
            .size = self->shader_binding_table.ray_gen_size,
        };
        VkStridedDeviceAddressRegionKHR miss_shader_binding_table = {
            .deviceAddress = shader_binding_table_miss_device_address(
                &self->shader_binding_table),
            .stride = self->shader_binding_table.miss_entry_size,
            .size = self->shader_binding_table.miss_size,
        };
        VkStridedDeviceAddressRegionKHR hit_shader_binding_table = {
            .deviceAddress = shader_binding_table_hit_group_device_address(
                &self->shader_binding_table),
            .stride = self->shader_binding_table.hit_group_entry_size,
            .size = self->shader_binding_table.hit_group_size,
        };

        VkStridedDeviceAddressRegionKHR callable_shader_binding_table = {0};

        self->device->procedures.vkCmdTraceRaysKHR(
            command_buffer,
            &ray_gen_shader_binding_table,
            &miss_shader_binding_table,
            &hit_shader_binding_table,
            &callable_shader_binding_table,
            extent.width,
            extent.height,
            1);

        image_memory_barrier_insert(command_buffer,
                                    self->output_image.handle,
                                    sub_resource_range,
                                    VK_ACCESS_SHADER_WRITE_BIT,
                                    VK_ACCESS_TRANSFER_READ_BIT,
                                    VK_IMAGE_LAYOUT_GENERAL,
                                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        image_memory_barrier_insert(command_buffer,
                                    self->swapchain.images[image_index],
                                    sub_resource_range,
                                    0,
                                    VK_ACCESS_SHADER_WRITE_BIT,
                                    VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VkImageCopy copy_region = {
            .srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
            .srcOffset = {0, 0, 0},
            .dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
            .dstOffset = {0, 0, 0},
            .extent = {extent.width, extent.height, 1},
        };

        vkCmdCopyImage(command_buffer,
                       self->output_image.handle,
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       self->swapchain.images[image_index],
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1,
                       &copy_region);

        image_memory_barrier_insert(command_buffer,
                                    self->swapchain.images[image_index],
                                    sub_resource_range,
                                    VK_ACCESS_TRANSFER_WRITE_BIT,
                                    0,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }
#else
    VkClearValue clear_values[] = {
        (VkClearValue){.color = {{0.0f, 0.0f, 0.0f, 1.0f}}},
        (VkClearValue){.depthStencil = {1.0f, 0}},
    };

    VkRenderPassBeginInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = self->graphics_pipeline->render_pass->handle,
        .framebuffer = self->framebuffers[image_index].handle,
        .renderArea =
            {
                .offset = {0, 0},
                .extent = self->swapchain.extent,
            },
        .clearValueCount = ARRAY_SIZE(clear_values),
        .pClearValues = clear_values,
    };

    vkCmdBeginRenderPass(command_buffer,
                         &render_pass_info,
                         VK_SUBPASS_CONTENTS_INLINE);
    {
        VkDescriptorSet descriptor_sets[] = {
            self->graphics_pipeline->descriptor_sets[self->current_frame]};
        VkBuffer vertex_buffers[] = {self->scene->vertex_buffer.handle};
        const VkBuffer index_buffer = self->scene->index_buffer.handle;
        VkDeviceSize offsets[] = {0};

        vkCmdBindPipeline(command_buffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          self->graphics_pipeline->handle);
        vkCmdBindDescriptorSets(command_buffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                self->graphics_pipeline->pipeline_layout.handle,
                                0,
                                1,
                                descriptor_sets,
                                0,
                                NULL);
        vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
        vkCmdBindIndexBuffer(command_buffer,
                             index_buffer,
                             0,
                             VK_INDEX_TYPE_UINT32);

        u32 vertex_offset = 0;
        u32 index_offset = 0;

        for (u32 i = 0; i < darray_length(self->scene->models); i++) {
            Model *model = &self->scene->models[i];
            u32 vertex_count = darray_length(model->vertices);
            u32 index_count = darray_length(model->indices);

            vkCmdDrawIndexed(command_buffer,
                             index_count,
                             1,
                             index_offset,
                             vertex_offset,
                             0);

            vertex_offset += vertex_count;
            index_offset += index_count;
        }
    }
    vkCmdEndRenderPass(command_buffer);

#endif

    command_buffer_end(&self->command_buffers, self->current_frame);

    mat4s model_view = glms_lookat((vec3s){{13, 2, 3}},
                                   (vec3s){{0, 0, 0}},
                                   (vec3s){{0, 1, 0}});
    mat4s projection = glms_perspective(glm_rad(20.0),
                                        (f32)self->swapchain.extent.width /
                                            (f32)self->swapchain.extent.height,
                                        0.1f,
                                        10000.0f);
    projection.m11 *= -1;

    UniformBufferObject ubo = {
        .model_view = model_view,
        .projection = projection,
        .model_view_inverse = mat4_inv(model_view),
        .projection_inverse = mat4_inv(projection),
        .aperture = 0.1f,
        .focus_distance = 10.0f,
        .random_seed = 1,
        .total_number_of_samples = self->total_number_of_samples,
        .number_of_samples = self->number_of_samples,
        .number_of_bounces = 16,
        .show_heat_map = false,
        .heatmap_scale = 1.0,
        .has_sky = true,
    };

    uniform_buffer_set_value(&self->uniform_buffers[self->current_frame], ubo);

    VkCommandBuffer command_buffers[] = {command_buffer};
    VkSemaphore wait_semaphores[] = {image_available_semaphore};
    VkPipelineStageFlags wait_stages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signal_semaphores[] = {render_finished_semaphore};

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = wait_semaphores,
        .pWaitDstStageMask = wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = command_buffers,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signal_semaphores,
    };

    fence_reset(in_flight_fence);

    vulkan_check(vkQueueSubmit(self->device->graphics_queue,
                               1,
                               &submit_info,
                               in_flight_fence->handle),
                 "submit draw command buffer");

    VkSwapchainKHR swapchains[] = {self->swapchain.handle};
    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signal_semaphores,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = &image_index,
    };

    result = vkQueuePresentKHR(self->device->present_queue, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreate_swapchain(self);
        return;
    }

    if (result != VK_SUCCESS) {
        LOG_FATAL("failed to present next image (%s)",
                  vulkan_result_to_string(result));
        exit(EXIT_FAILURE);
    }

    self->current_frame =
        (self->current_frame + 1) % darray_length(self->in_flight_fences);
}

static VkPhysicalDevice choose_physical_device(Application *application) {
    darray(VkPhysicalDevice) physical_devices =
        instance_physical_devices(&application->instance);

    VkPhysicalDevice found_device = VK_NULL_HANDLE;
    for (u32 i = 0; i < darray_length(physical_devices); i++) {
        VkPhysicalDevice device = physical_devices[i];

        VkPhysicalDeviceProperties2 props = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        };

        vkGetPhysicalDeviceProperties2(device, &props);

        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceFeatures(device, &device_features);

        if (!device_features.geometryShader) {
            continue;
        }

        u32 extension_count;
        vkEnumerateDeviceExtensionProperties(device,
                                             NULL,
                                             &extension_count,
                                             NULL);
        VkExtensionProperties extensions[extension_count];
        vkEnumerateDeviceExtensionProperties(device,
                                             NULL,
                                             &extension_count,
                                             extensions);

        b8 has_ray_tracing = false;
        for (u32 j = 0; j < extension_count; j++) {
            if (strcmp(extensions[j].extensionName,
                       VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0) {
                has_ray_tracing = true;
                break;
            }
        }
        if (!has_ray_tracing) {
            continue;
        }

        u32 queue_family_count;
        vkGetPhysicalDeviceQueueFamilyProperties(device,
                                                 &queue_family_count,
                                                 NULL);
        VkQueueFamilyProperties queue_families[queue_family_count];
        vkGetPhysicalDeviceQueueFamilyProperties(device,
                                                 &queue_family_count,
                                                 queue_families);

        b8 has_graphics_queue = false;
        for (u32 j = 0; j < queue_family_count; j++) {
            if (queue_families[j].queueCount > 0 &&
                queue_families[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                has_graphics_queue = true;
                break;
            }
        }
        if (!has_graphics_queue) {
            continue;
        }

        LOG_DEBUG("chose physical device: %s", props.properties.deviceName);

        found_device = device;
        break;
    }

    if (found_device == VK_NULL_HANDLE) {
        LOG_FATAL("cannot find a suitable device");
        exit(EXIT_FAILURE);
    }

    return found_device;
}

static void create_swapchain(Application *self) {
    while (window_is_minimized(self->window)) {
        window_wait_for_events();
    }

    self->swapchain = swapchain_new(self->device, self->present_mode);
    self->depth_buffer =
        depth_buffer_new(&self->command_pool, self->swapchain.extent);

    self->image_available_semaphores = darray_new(Semaphore);
    self->render_finished_semaphores = darray_new(Semaphore);
    self->in_flight_fences = darray_new(Fence);
    self->uniform_buffers = darray_new(UniformBuffer);

    for (u32 i = 0; i != darray_length(self->swapchain.image_views); i++) {
        darray_push(self->image_available_semaphores,
                    semaphore_new(self->device));
        darray_push(self->render_finished_semaphores,
                    semaphore_new(self->device));
        darray_push(self->in_flight_fences, fence_new(self->device, true));
        darray_push(self->uniform_buffers, uniform_buffer_new(self->device));
    }

    self->graphics_pipeline = malloc(sizeof(GraphicsPipeline));
    *self->graphics_pipeline = graphics_pipeline_new(&self->swapchain,
                                                     &self->depth_buffer,
                                                     self->uniform_buffers,
                                                     self->scene,
                                                     false);

    self->framebuffers = darray_new(Framebuffer);
    for (u32 i = 0; i < darray_length(self->swapchain.image_views); i++) {
        darray_push(self->framebuffers,
                    framebuffer_new(&self->swapchain.image_views[i],
                                    self->graphics_pipeline->render_pass));
    }

    self->command_buffers =
        command_buffers_new(&self->command_pool,
                            darray_length(self->swapchain.image_views));

    self->accumulation_image = image_new(self->device,
                                         self->swapchain.extent,
                                         VK_FORMAT_R32G32B32A32_SFLOAT,
                                         VK_IMAGE_TILING_OPTIMAL,
                                         VK_IMAGE_USAGE_STORAGE_BIT);
    self->accumulation_image_memory =
        image_allocate_memory(&self->accumulation_image,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    self->accumulation_image_view =
        image_view_new(self->device,
                       self->accumulation_image.handle,
                       VK_FORMAT_R32G32B32A32_SFLOAT,
                       VK_IMAGE_ASPECT_COLOR_BIT);

    self->output_image =
        image_new(self->device,
                  self->swapchain.extent,
                  self->swapchain.format,
                  VK_IMAGE_TILING_OPTIMAL,
                  VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    self->output_image_memory =
        image_allocate_memory(&self->output_image,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    self->output_image_view = image_view_new(self->device,
                                             self->output_image.handle,
                                             self->swapchain.format,
                                             VK_IMAGE_ASPECT_COLOR_BIT);

    self->ray_tracing_pipeline =
        ray_tracing_pipeline_new(&self->swapchain,
                                 self->top_as,
                                 &self->accumulation_image_view,
                                 &self->output_image_view,
                                 self->uniform_buffers,
                                 self->scene);

    darray(ShaderBindingTableEntry) ray_gen_programs =
        darray_new(ShaderBindingTableEntry);
    darray(ShaderBindingTableEntry) miss_programs =
        darray_new(ShaderBindingTableEntry);
    darray(ShaderBindingTableEntry) hit_groups =
        darray_new(ShaderBindingTableEntry);
    ShaderBindingTableEntry ray_gen_entry = {
        .group_index = self->ray_tracing_pipeline.ray_gen_index,
    };
    darray_push(ray_gen_programs, ray_gen_entry);
    ShaderBindingTableEntry miss_entry = {
        .group_index = self->ray_tracing_pipeline.miss_index,
    };
    darray_push(miss_programs, miss_entry);
    ShaderBindingTableEntry triangle_hit_group_entry = {
        .group_index = self->ray_tracing_pipeline.triangle_hit_group_index,
    };
    darray_push(hit_groups, triangle_hit_group_entry);

    self->shader_binding_table =
        shader_binding_table_new(self->device,
                                 &self->ray_tracing_pipeline,
                                 ray_gen_programs,
                                 miss_programs,
                                 hit_groups);

    darray_destroy(ray_gen_programs);
    darray_destroy(miss_programs);
    darray_destroy(hit_groups);
}

static void delete_swapchain(Application *self) {
    shader_binding_table_destroy(&self->shader_binding_table);
    ray_tracing_pipeline_destroy(&self->ray_tracing_pipeline);

    image_view_destroy(&self->output_image_view);
    image_destroy(&self->output_image);
    device_memory_destroy(&self->output_image_memory);
    image_view_destroy(&self->accumulation_image_view);
    image_destroy(&self->accumulation_image);
    device_memory_destroy(&self->accumulation_image_memory);

    command_buffers_destroy(&self->command_buffers);
    for (u32 i = 0; i < darray_length(self->framebuffers); i++) {
        framebuffer_destroy(&self->framebuffers[i]);
    }
    darray_destroy(self->framebuffers);

    graphics_pipeline_destroy(self->graphics_pipeline);
    free(self->graphics_pipeline);

    for (u32 i = 0; i < darray_length(self->in_flight_fences); i++) {
        fence_destroy(&self->in_flight_fences[i]);
        semaphore_destroy(&self->render_finished_semaphores[i]);
        semaphore_destroy(&self->image_available_semaphores[i]);
    }
    darray_destroy(self->in_flight_fences);
    darray_destroy(self->render_finished_semaphores);
    darray_destroy(self->image_available_semaphores);

    depth_buffer_destroy(&self->depth_buffer);
    swapchain_destroy(&self->swapchain);
}

static void recreate_swapchain(Application *self) {
    device_wait_idle(self->device);
    delete_swapchain(self);
    create_swapchain(self);
}
