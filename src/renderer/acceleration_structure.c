#include "acceleration_structure.h"
#include "renderer/ray_tracing_properties.h"
#include "vulkan/vulkan_core.h"

void acceleration_structure_get_build_sizes(
    const Device *device,
    VkAccelerationStructureBuildSizesInfoKHR *build_sizes_info,
    const VkAccelerationStructureBuildGeometryInfoKHR *build_geometry_info,
    const RayTracingProperties *ray_tracing_properties,
    const u32 *max_primitives_counts) {
    build_sizes_info->sType =
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

    device->procedures.vkGetAccelerationStructureBuildSizesKHR(
        device->handle,
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        build_geometry_info,
        max_primitives_counts,
        build_sizes_info);

    u64 acceleration_structure_alignment = 256;
    u64 scratch_alignment = ray_tracing_properties->acceleration_props
                                .minAccelerationStructureScratchOffsetAlignment;

#define ROUND_UP(value, granularity)                                           \
    ((((value) + (granularity) - 1) / (granularity)) * (granularity))

    build_sizes_info->accelerationStructureSize =
        ROUND_UP(build_sizes_info->accelerationStructureSize,
                 acceleration_structure_alignment);
    build_sizes_info->buildScratchSize =
        ROUND_UP(build_sizes_info->buildScratchSize, scratch_alignment);
}

VkAccelerationStructureKHR acceleration_structure_new(
    const Device *device,
    const VkAccelerationStructureBuildGeometryInfoKHR *build_geometry_info,
    const VkAccelerationStructureBuildSizesInfoKHR *build_sizes_info,
    const Buffer *result_buffer,
    VkDeviceSize result_offset) {
    VkAccelerationStructureCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
        .type = build_geometry_info->type,
        .size = build_sizes_info->accelerationStructureSize,
        .buffer = result_buffer->handle,
        .offset = result_offset,
    };

    VkAccelerationStructureKHR result;

    vulkan_check(
        device->procedures.vkCreateAccelerationStructureKHR(device->handle,
                                                            &create_info,
                                                            NULL,
                                                            &result),
        "create acceleration structure");

    return result;
}

void acceleration_structure_destroy(const Device *device,
                                    VkAccelerationStructureKHR *handle) {
    if (handle != NULL) {
        device->procedures.vkDestroyAccelerationStructureKHR(device->handle,
                                                             *handle,
                                                             NULL);
        *handle = NULL;
    }
}

void acceleration_structure_memory_barrier(VkCommandBuffer command_buffer) {
    VkMemoryBarrier memory_barrier = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR |
                         VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,
        .dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR |
                         VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,
    };

    vkCmdPipelineBarrier(command_buffer,
                         VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                         VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                         0,
                         1,
                         &memory_barrier,
                         0,
                         NULL,
                         0,
                         NULL);
}
