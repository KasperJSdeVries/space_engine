#include "top_level_acceleration_structure.h"
#include "renderer/acceleration_structure.h"
#include "renderer/bottom_level_acceleration_structure.h"
#include "vulkan/vulkan_core.h"

TopLevelAccelerationStructure top_level_acceleration_structure_new(
    const Device *device,
    VkDeviceAddress instance_address,
    u32 instance_count) {
    TopLevelAccelerationStructure self = {
        .device = device,
        .flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
        .instance_count = instance_count,
        .ray_tracing_properties =
            ray_tracing_properties_fetch(device->physical_device),
    };

    self.instances.sType =
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    self.instances.arrayOfPointers = VK_FALSE;
    self.instances.data.deviceAddress = instance_address;

    self.top_as_geometry.sType =
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    self.top_as_geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    self.top_as_geometry.geometry.instances = self.instances;

    self.build_geometry_info.sType =
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    self.build_geometry_info.flags = self.flags;
    self.build_geometry_info.geometryCount = 1;
    self.build_geometry_info.pGeometries = &self.top_as_geometry;
    self.build_geometry_info.mode =
        VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    self.build_geometry_info.type =
        VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    self.build_geometry_info.srcAccelerationStructure = NULL;

    acceleration_structure_get_build_sizes(device,
                                           &self.build_sizes_info,
                                           &self.build_geometry_info,
                                           &self.ray_tracing_properties,
                                           &instance_count);

    return self;
}

void top_level_acceleration_structure_destroy(
    TopLevelAccelerationStructure *self) {
    acceleration_structure_destroy(self->device, &self->handle);
}

void top_level_acceleration_structure_generate(
    TopLevelAccelerationStructure *self,
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

    VkAccelerationStructureBuildRangeInfoKHR offset_info = {
        .primitiveCount = self->instance_count,
    };

    const VkAccelerationStructureBuildRangeInfoKHR *build_offset_info =
        &offset_info;

    self->build_geometry_info.pGeometries = &self->top_as_geometry;
    self->build_geometry_info.dstAccelerationStructure = self->handle;
    self->build_geometry_info.scratchData.deviceAddress =
        buffer_get_device_address(scratch_buffer) + scratch_offset;

    self->device->procedures.vkCmdBuildAccelerationStructuresKHR(
        command_buffer,
        1,
        &self->build_geometry_info,
        &build_offset_info);
}

VkAccelerationStructureInstanceKHR
top_level_acceleration_structure_create_instance(
    const BottomLevelAccelerationStructure *bottom_level_as,
    mat4s transform,
    u32 instance_id,
    u32 hit_group_id) {
    const Device *device = bottom_level_as->device;

    VkAccelerationStructureDeviceAddressInfoKHR address_info = {
        .sType =
            VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
        .accelerationStructure = bottom_level_as->handle,
    };

    VkDeviceAddress address =
        bottom_level_as->device->procedures
            .vkGetAccelerationStructureDeviceAddressKHR(device->handle,
                                                        &address_info);

    VkAccelerationStructureInstanceKHR instance = {
        .instanceCustomIndex = instance_id,
        .mask = 0xff,
        .instanceShaderBindingTableRecordOffset = hit_group_id,
        .flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
        .accelerationStructureReference = address,
    };

    memcpy(&instance.transform, &transform, sizeof(instance.transform));

    return instance;
}
