
#ifndef ACCELERATION_STRUCTURE_H
#define ACCELERATION_STRUCTURE_H

#include "renderer/buffer.h"
#include "renderer/device.h"
#include "renderer/ray_tracing_properties.h"
#include "vulkan/vulkan_core.h"

void acceleration_structure_get_build_sizes(
    const Device *device,
    VkAccelerationStructureBuildSizesInfoKHR *build_sizes_info,
    const VkAccelerationStructureBuildGeometryInfoKHR *build_geometry_info,
    const RayTracingProperties *ray_tracing_properties,
    const u32 *max_primitives_counts);

VkAccelerationStructureKHR acceleration_structure_new(
    const Device *device,
    const VkAccelerationStructureBuildGeometryInfoKHR *build_geometry_info,
    const VkAccelerationStructureBuildSizesInfoKHR *build_sizes_info,
    const Buffer *result_buffer,
    VkDeviceSize result_offset);
void acceleration_structure_destroy(const Device *device,
                                    VkAccelerationStructureKHR *handle);

void acceleration_structure_memory_barrier(VkCommandBuffer command_buffer);

void cmdBuildAccelerationStructuresKHR(
    VkDevice device,
    VkCommandBuffer commandBuffer,
    uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos);

#endif // ACCELERATION_STRUCTURE_H
