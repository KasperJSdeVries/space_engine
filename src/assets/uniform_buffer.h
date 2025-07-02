#ifndef UNIFORM_BUFFER_H
#define UNIFORM_BUFFER_H

#include "cglm/types-struct.h"
#include "core/defines.h"
#include "renderer/buffer.h"
#include "renderer/device_memory.h"

typedef struct {
    mat4s model_view;
    mat4s projection;
    mat4s model_view_inverse;
    mat4s projection_inverse;
    f32 aperture;
    f32 focus_distance;
    f32 heatmap_scale;
    u32 total_number_of_samples;
    u32 number_of_samples;
    u32 number_of_bounces;
    u32 random_seed;
    b32 has_sky;
    b32 show_heat_map;
} UniformBufferObject;

typedef struct {
    Buffer buffer;
    DeviceMemory memory;
} UniformBuffer;

UniformBuffer uniform_buffer_new(const Device *device);
void uniform_buffer_destroy(UniformBuffer *self);

void uniform_buffer_set_value(UniformBuffer *self, UniformBufferObject value);

#endif // UNIFORM_BUFFER_H
