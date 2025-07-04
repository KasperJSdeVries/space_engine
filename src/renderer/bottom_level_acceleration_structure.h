#ifndef BOTTOM_LEVEL_ACCELERATION_STRUCTURE_H
#define BOTTOM_LEVEL_ACCELERATION_STRUCTURE_H

#include "renderer/bottom_level_geometry.h"
#include "renderer/device.h"
#include "renderer/ray_tracing_properties.h"
#include "vulkan/vulkan_core.h"

typedef struct {
    const Device *device;
    VkAccelerationStructureKHR handle;
    BottomLevelGeometry geometries;
    VkAccelerationStructureBuildGeometryInfoKHR build_geometry_info;
    VkAccelerationStructureBuildSizesInfoKHR build_sizes_info;
    RayTracingProperties ray_tracing_properties;
    VkBuildAccelerationStructureFlagsKHR flags;
} BottomLevelAccelerationStructure;

BottomLevelAccelerationStructure bottom_level_acceleration_structure_new(
    const Device *device,
    BottomLevelGeometry geometry);
void bottom_level_acceleration_structure_destroy(
    BottomLevelAccelerationStructure *self);

void bottom_level_acceleration_structure_generate(
    BottomLevelAccelerationStructure *self,
    VkCommandBuffer command_buffer,
    const Buffer *scratch_buffer,
    VkDeviceSize scratch_offset,
    const Buffer *result_buffer,
    VkDeviceSize result_offset);

#endif // BOTTOM_LEVEL_ACCELERATION_STRUCTURE_H
