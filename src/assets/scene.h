#ifndef SCENE_H
#define SCENE_H

#include "assets/model.h"
#include "assets/texture.h"
#include "assets/texture_image.h"
#include "containers/darray.h"
#include "renderer/buffer.h"
#include "renderer/device_memory.h"

typedef struct {
    darray(Model) models;
    darray(Texture) textures;
    Buffer vertex_buffer;
    DeviceMemory vertex_buffer_memory;
    Buffer index_buffer;
    DeviceMemory index_buffer_memory;
    Buffer material_buffer;
    DeviceMemory material_buffer_memory;
    Buffer offset_buffer;
    DeviceMemory offset_buffer_memory;
    darray(TextureImage) texture_images;
    darray(VkImageView) texture_image_views;
    darray(VkSampler) texture_samplers;
} Scene;

Scene scene_new(darray(Model) models, darray(Texture) textures);
void scene_destroy(Scene *self);

void scene_generate_buffers(Scene *self, CommandPool *command_pool);
void scene_destroy_buffers(Scene *self);

#endif // SCENE_H
