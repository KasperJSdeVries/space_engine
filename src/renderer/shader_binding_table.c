#include "shader_binding_table.h"
#include "containers/darray.h"
#include "renderer/buffer.h"
#include "renderer/device_memory.h"
#include "renderer/ray_tracing_properties.h"
#include "renderer/vulkan.h"
#include "vulkan/vulkan_core.h"

static u64 get_entry_size(RayTracingProperties ray_tracing_properties,
                          darray(ShaderBindingTableEntry) entries);
static u64 copy_shader_data(u8 *dst,
                            RayTracingProperties ray_tracing_properties,
                            darray(ShaderBindingTableEntry) entries,
                            u64 entry_size,
                            const u8 *shader_handle_storage);

ShaderBindingTable shader_binding_table_new(
    const Device *device,
    const RayTracingPipeline *ray_tracing_pipeline,
    darray(ShaderBindingTableEntry) ray_gen_programs,
    darray(ShaderBindingTableEntry) miss_programs,
    darray(ShaderBindingTableEntry) hit_groups) {
    ShaderBindingTable self = {0};

    RayTracingProperties ray_tracing_props =
        ray_tracing_properties_fetch(device->physical_device);

    self.ray_gen_entry_size =
        get_entry_size(ray_tracing_props, ray_gen_programs);
    self.miss_entry_size = get_entry_size(ray_tracing_props, miss_programs);
    self.hit_group_entry_size = get_entry_size(ray_tracing_props, hit_groups);

    self.ray_gen_offset = 0;
    self.miss_offset =
        darray_length(ray_gen_programs) * self.ray_gen_entry_size;
    self.hit_group_offset =
        self.miss_offset + self.miss_entry_size * darray_length(miss_programs);

    self.ray_gen_size =
        darray_length(ray_gen_programs) * self.ray_gen_entry_size;
    self.miss_size = darray_length(miss_programs) * self.miss_entry_size;
    self.hit_group_size = darray_length(hit_groups) * self.hit_group_entry_size;

    u64 total_size = darray_length(ray_gen_programs) * self.ray_gen_entry_size +
                     darray_length(miss_programs) * self.miss_entry_size +
                     darray_length(hit_groups) * self.hit_group_entry_size;

    self.buffer = buffer_new(device,
                             total_size,
                             VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                 VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR);
    self.buffer_memory =
        buffer_allocate_memory(&self.buffer,
                               VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    u32 handle_size = ray_tracing_props.pipeline_props.shaderGroupHandleSize;
    u64 group_count = darray_length(ray_gen_programs) +
                      darray_length(miss_programs) + darray_length(hit_groups);
    u8 shader_handle_storage[group_count * handle_size];

    vulkan_check(device->procedures.vkGetRayTracingShaderGroupHandlesKHR(
                     device->handle,
                     ray_tracing_pipeline->handle,
                     0,
                     group_count,
                     sizeof(shader_handle_storage),
                     shader_handle_storage),
                 "get ray tracing shader group handles");

    u8 *data = device_memory_map(&self.buffer_memory, 0, total_size);
    data += copy_shader_data(data,
                             ray_tracing_props,
                             ray_gen_programs,
                             self.ray_gen_entry_size,
                             shader_handle_storage);
    data += copy_shader_data(data,
                             ray_tracing_props,
                             miss_programs,
                             self.miss_entry_size,
                             shader_handle_storage);
    copy_shader_data(data,
                     ray_tracing_props,
                     hit_groups,
                     self.hit_group_entry_size,
                     shader_handle_storage);
    device_memory_unmap(&self.buffer_memory);

    return self;
}

void shader_binding_table_destroy(ShaderBindingTable *self) {
    buffer_destroy(&self->buffer);
    device_memory_destroy(&self->buffer_memory);
}

VkDeviceAddress shader_binding_table_ray_gen_device_address(
    const ShaderBindingTable *self) {
    return buffer_get_device_address(&self->buffer) + self->ray_gen_offset;
}

VkDeviceAddress shader_binding_table_miss_device_address(
    const ShaderBindingTable *self) {
    return buffer_get_device_address(&self->buffer) + self->miss_offset;
}
VkDeviceAddress shader_binding_table_hit_group_device_address(
    const ShaderBindingTable *self) {
    return buffer_get_device_address(&self->buffer) + self->hit_group_offset;
}

static u64 get_entry_size(RayTracingProperties ray_tracing_properties,
                          darray(ShaderBindingTableEntry) entries) {
    u64 max_args = 0;

    for (u32 i = 0; i < darray_length(entries); i++) {
        max_args = MAX(max_args, entries[i].inline_data_size);
    }

    return (ray_tracing_properties.pipeline_props.shaderGroupHandleSize +
            ray_tracing_properties.pipeline_props.shaderGroupBaseAlignment -
            1) &
           ~(ray_tracing_properties.pipeline_props.shaderGroupBaseAlignment -
             1);
}

static u64 copy_shader_data(u8 *dst,
                            RayTracingProperties ray_tracing_properties,
                            darray(ShaderBindingTableEntry) entries,
                            u64 entry_size,
                            const u8 *shader_handle_storage) {
    u64 handle_size =
        ray_tracing_properties.pipeline_props.shaderGroupHandleSize;

    u8 *ptr = dst;

    for (u32 i = 0; i < darray_length(entries); i++) {
        memcpy(ptr,
               shader_handle_storage + entries[i].group_index * handle_size,
               handle_size);
        if (entries[i].inline_data != NULL) {
            memcpy(ptr + handle_size,
                   entries[i].inline_data,
                   entries[i].inline_data_size);
        }

        ptr += entry_size;
    }

    return darray_length(entries) * entry_size;
}
