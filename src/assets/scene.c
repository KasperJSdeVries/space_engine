#include "scene.h"
#include "assets/material.h"
#include "assets/texture_image.h"
#include "containers/darray.h"
#include "renderer/buffer.h"
#include "vulkan/vulkan_core.h"

Scene scene_new(darray(Model) models, darray(Texture) textures) {
    return (Scene){
        .models = models,
        .textures = textures,
    };
}

void scene_destroy(Scene *self) { scene_destroy_buffers(self); }

void scene_generate_buffers(Scene *self, CommandPool *command_pool) {
    darray(Vertex) vertices = darray_new(Vertex);
    darray(u32) indices = darray_new(u32);
    darray(Material) materials = darray_new(Material);
    darray(ivec2s) offsets = darray_new(ivec2s);

    for (u32 m = 0; m < darray_length(self->models); m++) {
        const Model *model = &self->models[m];

        u32 index_offset = darray_length(indices);
        u32 vertex_offset = darray_length(vertices);
        u32 material_offset = darray_length(materials);

        ivec2s model_offsets = {{index_offset, vertex_offset}};
        darray_push(offsets, model_offsets);

        darray_append(vertices, model->vertices);
        darray_append(indices, model->indices);
        darray_append(materials, model->materials);

        for (u64 i = vertex_offset; i != darray_length(vertices); i++) {
            vertices[i].material_index += material_offset;
        }
    }

    VkBufferUsageFlags flags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                               VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    create_device_buffer(
        command_pool,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
            flags,
        vertices,
        &self->vertex_buffer,
        &self->vertex_buffer_memory);
    create_device_buffer(
        command_pool,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
            flags,
        indices,
        &self->index_buffer,
        &self->index_buffer_memory);
    create_device_buffer(command_pool,
                         flags,
                         materials,
                         &self->material_buffer,
                         &self->material_buffer_memory);
    create_device_buffer(command_pool,
                         flags,
                         offsets,
                         &self->offset_buffer,
                         &self->offset_buffer_memory);

    self->texture_images = darray_new(TextureImage);
    self->texture_image_views = darray_new(VkImageView);
    self->texture_samplers = darray_new(VkSampler);
    for (u64 i = 0; i < darray_length(self->textures); i++) {
        darray_push(self->texture_images,
                    texture_image_new(command_pool, &self->textures[i]));
        darray_push(self->texture_image_views,
                    self->texture_images->image_view.handle);
        darray_push(self->texture_samplers,
                    self->texture_images->sampler.handle);
    }
}

void scene_destroy_buffers(Scene *self) {
    if (self->texture_images != NULL) {

        for (u64 i = 0; i < darray_length(self->texture_images); i++) {
            texture_image_destroy(&self->texture_images[i]);
        }

        darray_destroy(self->texture_images);
        darray_destroy(self->texture_image_views);
        darray_destroy(self->texture_samplers);

        buffer_destroy(&self->offset_buffer);
        device_memory_destroy(&self->offset_buffer_memory);
        buffer_destroy(&self->material_buffer);
        device_memory_destroy(&self->material_buffer_memory);
        buffer_destroy(&self->index_buffer);
        device_memory_destroy(&self->index_buffer_memory);
        buffer_destroy(&self->vertex_buffer);
        device_memory_destroy(&self->vertex_buffer_memory);
    }
}
