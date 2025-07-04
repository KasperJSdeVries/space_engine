#ifndef TOP_LEVEL_ACCELERATION_STRUCTURE_H
#define TOP_LEVEL_ACCELERATION_STRUCTURE_H

#include "renderer/bottom_level_acceleration_structure.h"
#include "renderer/buffer.h"
#include "renderer/device.h"
#include "renderer/ray_tracing_properties.h"
#include "vulkan/vulkan_core.h"

typedef struct {
    const Device *device;
    VkAccelerationStructureKHR handle;
    VkAccelerationStructureGeometryInstancesDataKHR instances;
    VkAccelerationStructureGeometryKHR top_as_geometry;
    VkAccelerationStructureBuildGeometryInfoKHR build_geometry_info;
    VkAccelerationStructureBuildSizesInfoKHR build_sizes_info;
    RayTracingProperties ray_tracing_properties;
    VkBuildAccelerationStructureFlagsKHR flags;
    u32 instance_count;
} TopLevelAccelerationStructure;

TopLevelAccelerationStructure top_level_acceleration_structure_new(
    const Device *device,
    VkDeviceAddress instance_address,
    u32 instance_count);
void top_level_acceleration_structure_destroy(
    TopLevelAccelerationStructure *self);

void top_level_acceleration_structure_generate(
    TopLevelAccelerationStructure *self,
    VkCommandBuffer command_buffer,
    const Buffer *scratch_buffer,
    VkDeviceSize scratch_offset,
    const Buffer *result_buffer,
    VkDeviceSize result_offset);

VkAccelerationStructureInstanceKHR
top_level_acceleration_structure_create_instance(
    const BottomLevelAccelerationStructure *bottom_level_as,
    mat4s transform,
    u32 instance_id,
    u32 hit_group_id);

void acceleration_structure_memory_barrier(VkCommandBuffer command_buffer);

#endif // TOP_LEVEL_ACCELERATION_STRUCTURE_H
