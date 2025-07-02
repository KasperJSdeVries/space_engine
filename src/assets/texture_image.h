#ifndef TEXTURE_IMAGE_H
#define TEXTURE_IMAGE_H

#include "assets/texture.h"
#include "renderer/command_pool.h"
#include "renderer/device_memory.h"
#include "renderer/image.h"
#include "renderer/image_view.h"
#include "renderer/sampler.h"

typedef struct {
    Image image;
    DeviceMemory image_memory;
    ImageView image_view;
    Sampler sampler;
} TextureImage;

TextureImage texture_image_new(CommandPool *command_pool,
                               const Texture *texture);
void texture_image_destroy(TextureImage *self);

#endif // TEXTURE_IMAGE_H
