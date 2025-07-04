#include "bottom_level_acceleration_structure.h"
#include "renderer/acceleration_structure.h"
#include "renderer/buffer.h"
#include "renderer/ray_tracing_properties.h"
#include "vulkan/vulkan_core.h"

BottomLevelAccelerationStructure bottom_level_acceleration_structure_new(
    const Device *device,
    BottomLevelGeometry geometry) {
    BottomLevelAccelerationStructure self = {
        .device = device,
        .flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
        .geometries = geometry,
        .ray_tracing_properties =
            ray_tracing_properties_fetch(device->physical_device),
    };

    self.build_geometry_info.sType =
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    self.build_geometry_info.flags = self.flags;
    self.build_geometry_info.geometryCount =
        darray_length(self.geometries.geometry);
    self.build_geometry_info.pGeometries = self.geometries.geometry;
    self.build_geometry_info.mode =
        VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    self.build_geometry_info.type =
        VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    self.build_geometry_info.srcAccelerationStructure = NULL;

    u32 max_prim_count[darray_length(self.geometries.geometry)];

    for (u64 i = 0; i < darray_length(self.geometries.geometry); i++) {
        max_prim_count[i] = self.geometries.build_offset_info[i].primitiveCount;
    }

    acceleration_structure_get_build_sizes(device,
                                           &self.build_sizes_info,
                                           &self.build_geometry_info,
                                           &self.ray_tracing_properties,
                                           max_prim_count);

    return self;
}

void bottom_level_acceleration_structure_destroy(
    BottomLevelAccelerationStructure *self) {
    acceleration_structure_destroy(self->device, &self->handle);
}

void bottom_level_acceleration_structure_generate(
    BottomLevelAccelerationStructure *self,
    VkCommandBuffer command_buffer,
    const Buffer *scratch_buffer,
    VkDeviceSize scratch_offset,
    const Buffer *result_buffer,
    VkDeviceSize result_offset) {
    self->handle = acceleration_structure_new(self->device,
                                              &self->build_geometry_info,
                                              &self->build_sizes_info,
                                              result_buffer,
                                              result_offset);

    const VkAccelerationStructureBuildRangeInfoKHR *build_offset_info =
        self->geometries.build_offset_info;

    self->build_geometry_info.dstAccelerationStructure = self->handle;
    self->build_geometry_info.scratchData.deviceAddress =
        buffer_get_device_address(scratch_buffer) + scratch_offset;

    self->device->procedures.vkCmdBuildAccelerationStructuresKHR(
        command_buffer,
        1,
        &self->build_geometry_info,
        &build_offset_info);
}
