#ifndef SHADER_BINDING_TABLE_H
#define SHADER_BINDING_TABLE_H

#include "core/defines.h"
#include "renderer/buffer.h"
#include "renderer/device_memory.h"
#include "renderer/ray_tracing_pipeline.h"

typedef struct {
    u32 group_index;
    u8 *inline_data;
    u64 inline_data_size;
} ShaderBindingTableEntry;

typedef struct {
    u64 ray_gen_entry_size;
    u64 miss_entry_size;
    u64 hit_group_entry_size;

    u64 ray_gen_offset;
    u64 miss_offset;
    u64 hit_group_offset;

    u64 ray_gen_size;
    u64 miss_size;
    u64 hit_group_size;

    Buffer buffer;
    DeviceMemory buffer_memory;
} ShaderBindingTable;

ShaderBindingTable shader_binding_table_new(
    const Device *device,
    const RayTracingPipeline *ray_tracing_pipeline,
    darray(ShaderBindingTableEntry) ray_gen_programs,
    darray(ShaderBindingTableEntry) miss_programs,
    darray(ShaderBindingTableEntry) hit_groups);
void shader_binding_table_destroy(ShaderBindingTable *self);

VkDeviceAddress shader_binding_table_ray_gen_device_address(
    const ShaderBindingTable *self);
VkDeviceAddress shader_binding_table_miss_device_address(
    const ShaderBindingTable *self);
VkDeviceAddress shader_binding_table_hit_group_device_address(
    const ShaderBindingTable *self);

#endif // SHADER_BINDING_TABLE_H
